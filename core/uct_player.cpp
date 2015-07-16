#include "uct_player.h"
#include "random_player.h"
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace chrono;

uct_player::uct_player(double _think_time, const abstract_player* const _opp, bool opposite) :
    abstract_player(_opp, opposite),
    root(nullptr)
{
    think_time = duration<double>(_think_time);
    if (_think_time < 0) {
        throw invalid_argument("Think time can't be negative");
    }
}

uct_player::~uct_player()
{
    if (root) {
        delete root;
    }
}

bool uct_player::think_next_move(pos_move &_move, const board &)
{
    if (!root) {
        root = new node(new random_player(*this), new random_player(*opponent), true);
    }

    time_point<steady_clock> start = steady_clock::now();//steady_clock is best suitable for measuring intervals
    for (duration<double> elapsed = steady_clock::now() - start;
         elapsed < think_time;
         elapsed = steady_clock::now() - start)
    {
        if (!root->select()) {
            break;
        }
    }

    auto best_child = root->get_best_child();
    if (best_child != root->child_end()) {
        //this is the new tree root
        node* new_root = root->release_child(best_child);
        delete root;
        root = new_root;

        _move = root->get_our_move();
        return true;
    }
#ifdef _DEBUG
    cout << "The get_best_child() function returned nullptr" << endl;
#endif
    return false;
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
    } else {
        new_root = new node(new random_player(*this), new random_player(*opponent), true);
    }
    delete root;
    root = new_root;
}

int uct_player::get_total_simulations() const
{
    return node::get_total_simulations();
}
