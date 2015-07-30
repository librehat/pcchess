#include "uct_player.h"
#include "random_player.h"
#include "../core/game.h"
#include "serialization_export.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <chrono>

using namespace std;
using namespace chrono;

uct_player::uct_player(bool red) :
    abstract_player(red),
    root(nullptr)
{}

uct_player::~uct_player()
{
    if (root) {
        delete root;
    }
}

bool uct_player::think_next_move(pos_move &_move, const board &bd, uint8_t no_eat_half_rounds, const vector<pos_move> &)
{
    milliseconds think_time = milliseconds(game::step_time);
    steady_clock::time_point start = steady_clock::now();//steady_clock is best suitable for measuring intervals

    if (!root) {
        root = new node(game::generate_fen(bd), true, red_side, no_eat_half_rounds);
        node::set_root_depth(root);
    }

    for (milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
         elapsed < think_time;
         elapsed = duration_cast<milliseconds>(steady_clock::now() - start))
    {
        if (!root->select()) {
            break;
        }
    }

    auto best_child = root->get_best_child();
    if (best_child == root->child_end()) {
        return false;
    }

    _move = (*best_child)->get_move();
    node* new_root = root->release_child(best_child);
    node::set_root_depth(new_root);
    delete root;
    root = new_root;
/*
#if defined(_DEBUG)
    static int i = 0;
    if (i == 0) {
        start = steady_clock::now();
        ofstream fs("/tmp/t.xml");
        xml_archive_tree(fs, root);
        fs.close();
        cout << "xml_archive_tree took " << duration_cast<milliseconds>(steady_clock::now() - start).count() << " milliseconds" << endl;
    }
    i++;
#endif*/
    return true;
}

void uct_player::opponent_moved(const pos_move &m)
{
    if (!root) {
        return;
    }

    node *new_root = nullptr;
    auto root_iter = root->find_child(m);
    if (root_iter != root->child_end()) {
        new_root = root->release_child(root_iter);
        node::set_root_depth(new_root);
    }
    delete root;
    root = new_root;
}

int64_t uct_player::get_total_simulations() const
{
    return node::get_total_simulations();
}

node* uct_player::get_tree() const
{
    return root;
}

void uct_player::text_archive_tree(ostream &os, node *b)
{
    boost::archive::text_oarchive oa(os);
    oa << boost::serialization::make_nvp("tree", b);
}

void uct_player::xml_archive_tree(ostream &os, node *b)
{
    boost::archive::xml_oarchive oa(os);
    oa << boost::serialization::make_nvp("tree", b);
}
