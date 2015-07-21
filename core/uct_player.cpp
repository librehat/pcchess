#include "uct_player.h"
#include "random_player.h"
#include "game.h"
#include "serialization_export.h"
#include <iostream>
#include <stdexcept>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <chrono>

using namespace std;
using namespace chrono;

uct_player::uct_player(bool opposite) :
    abstract_player(opposite),
    root(nullptr)
{}

uct_player::~uct_player()
{
    if (root) {
        delete root;
    }
}

bool uct_player::think_next_move(pos_move &_move, const board &, const abstract_player &opponent)
{
    duration<double> think_time = duration<double>(game::step_time);
    time_point<steady_clock> start = steady_clock::now();//steady_clock is best suitable for measuring intervals{

    if (!root) {
        root = new node(new random_player(*this), new random_player(opponent), true);
    }

    for (duration<double> elapsed = steady_clock::now() - start;
         elapsed < think_time;
         elapsed = steady_clock::now() - start)
    {
        if (!root->select()) {
            break;
        }
    }

    auto best_child = root->get_best_child();
    if (best_child == root->child_end()) {
        return false;
    }

    //this is the new tree root
    node* new_root = root->release_child(best_child);
    delete root;
    root = new_root;

    _move = root->get_our_move();
    return true;
}

void uct_player::opponent_moved(const pos_move &m, const abstract_player &opponent)
{
    if (!root) {
        return;
    }

    node *new_root = nullptr;
    auto root_iter = root->find_child(m);
    if (root_iter != root->child_end()) {
        new_root = root->release_child(root_iter);
    } else {
        new_root = new node(new random_player(*this), new random_player(opponent), true);
    }
    delete root;
    root = new_root;
}

int uct_player::get_total_simulations() const
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
