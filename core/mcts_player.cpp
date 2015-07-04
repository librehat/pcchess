#include "mcts_player.h"
#include "random_player.h"
#include <iostream>

using namespace std;
using namespace chrono;

mcts_player::mcts_player(double _think_time, bool first_hand, const abstract_player* const _opp) :
    abstract_player(),
    opp(_opp),
    firsthand(first_hand),
    root(nullptr)
{
    think_time = duration<double>(_think_time);
}

mcts_player::~mcts_player()
{
    if (root) {
        delete root;
    }
}

bool mcts_player::think_next_move(pos_move &_move, const board &)
{
    if (!root) {
        root = new node(new random_player(*this), new random_player(*opp), firsthand);
    }

    time_point<steady_clock> start = steady_clock::now();//steady_clock is best suitable for measuring intervals
    for (duration<double> elapsed = steady_clock::now() - start;
         elapsed < think_time;
         elapsed = steady_clock::now() - start)
    {
        root->select();
        //TODO if the MCTS progress can't continue, we should break the loop immediately
    }

    node* best_child = root->get_best_child();
    if (best_child) {
        //this is the new tree root
        best_child->detach();
        delete root;
        root = best_child;

        _move = root->get_our_move();
        return true;
    }
#ifdef _DEBUG
    cout << "The get_best_child() function returned nullptr" << endl;
#endif
    return false;
}

void mcts_player::opponent_moved(const pos_move &m)
{
    node* new_root = root->find_child(m);
    if (new_root) {
        new_root->detach();
    } else {
        new_root = new node(new random_player(*this), new random_player(*opp), true);
    }
    delete root;
    root = new_root;
}
