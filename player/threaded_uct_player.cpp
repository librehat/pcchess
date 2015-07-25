#include "threaded_uct_player.h"
#include "threaded_node.h"
#include "random_player.h"
#include "../core/game.h"
#include <chrono>

using namespace std;
using namespace chrono;

threaded_uct_player::threaded_uct_player(bool opposite, int _threads) :
    uct_player(opposite),
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

bool threaded_uct_player::think_next_move(pos_move &_move, const board &, const abstract_player &opponent, unsigned int no_eat_half_rounds, const vector<pos_move> &banmoves)
{
    milliseconds think_time = milliseconds(game::step_time);
    steady_clock::time_point start = steady_clock::now();//steady_clock is best suitable for measuring intervals

    if (!root) {
        root = new threaded_node(new random_player(*this), new random_player(opponent), true, no_eat_half_rounds, banmoves);
    }

    for (milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
         elapsed < think_time;
         elapsed = duration_cast<milliseconds>(steady_clock::now() - start))
    {
        for (auto &&t : thread_vec) {
            t = thread(&node::select, root);
        }
        for (auto &&t : thread_vec) {
            t.join();
        }
    }

    auto best_child = root->get_best_child();
    if (best_child == root->child_end()) {
        return false;
    }

    node* new_root = root->release_child(best_child);
    delete root;
    root = new_root;

    _move = root->get_our_move();
    return true;
}

void threaded_uct_player::opponent_moved(const pos_move &m, const abstract_player &opponent, unsigned int no_eat_half_rounds)
{
    if (!root) {
        return;
    }

    node *new_root = nullptr;
    auto root_iter = root->find_child(m);
    if (root_iter != root->child_end()) {
        new_root = root->release_child(root_iter);
    } else {
        new_root = new threaded_node(new random_player(*this), new random_player(opponent), true, no_eat_half_rounds);
    }
    delete root;
    root = new_root;
}

int64_t threaded_uct_player::get_total_simulations() const
{
    return threaded_node::get_total_simulations();
}
