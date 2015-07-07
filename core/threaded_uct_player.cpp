#include "threaded_uct_player.h"
#include "threaded_node.h"
#include "random_player.h"

using namespace std;
using namespace chrono;

threaded_uct_player::threaded_uct_player(double _think_time, bool first_hand, const abstract_player * const _opp, int _threads) :
    uct_player(_think_time, first_hand, _opp),
    threads(_threads)
{
    if (threads <= 0) {
        threads = thread::hardware_concurrency();
    }
    if (threads == 0) {
        threads = 1;
    }
    thread_vec.resize(threads);
}

bool threaded_uct_player::think_next_move(pos_move &_move, const board &m_board)
{
    if (!root) {
        root = new threaded_node(new random_player(*this), new random_player(*opp), firsthand);
    }

    time_point<steady_clock> start = steady_clock::now();//steady_clock is best suitable for measuring intervals
    for (duration<double> elapsed = steady_clock::now() - start;
         elapsed < think_time;
         elapsed = steady_clock::now() - start)
    {
        for (auto &&t : thread_vec) {
            t = thread(&node::select, root);
        }
        for (auto &&t : thread_vec) {
            t.join();
        }
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
    return false;
}

void threaded_uct_player::opponent_moved(const pos_move &m)
{
    node* new_root = root->find_child(m);
    if (new_root) {
        new_root->detach();
    } else {
        new_root = new threaded_node(new random_player(*this), new random_player(*opp), true);
    }
    delete root;
    root = new_root;
}
