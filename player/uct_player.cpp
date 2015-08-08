#include "uct_player.h"
#include "random_player.h"
#include "../core/game.h"
#include "utils/serialization_export.hpp"
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
    selects(0)
{}

bool uct_player::think_next_move(pos_move &_move, const board &bd, uint8_t no_eat_half_rounds, const vector<pos_move> &)
{
    milliseconds think_time = milliseconds(game::step_time);
    steady_clock::time_point start = steady_clock::now();//steady_clock is best suitable for measuring intervals

    if (!root) {
        root = node::node_ptr(new node(game::generate_fen(bd), red_side, no_eat_half_rounds));
        node::set_root_depth(root);
    }

    for (milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
         elapsed < think_time;
         elapsed = duration_cast<milliseconds>(steady_clock::now() - start))
    {
        root->select();
        selects++;
    }

    auto best_child = root->get_best_child();
    if (best_child == root->child_end()) {
        return false;
    }

    _move = (*best_child)->get_move();
    node::node_ptr new_root = root->release_child(best_child);
    node::set_root_depth(new_root);
    root = new_root;

    return true;
}

void uct_player::opponent_moved(const pos_move &m)
{
    if (!root) {
        return;
    }

    node::node_ptr new_root;
    auto root_iter = root->find_child(m);
    if (root_iter != root->child_end()) {
        new_root = root->release_child(root_iter);
        node::set_root_depth(new_root);
    }
    root = new_root;
}

uint64_t uct_player::get_total_simulations() const
{
    return selects;
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
