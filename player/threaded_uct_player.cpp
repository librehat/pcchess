#include "threaded_uct_player.h"
#include "threaded_node.h"
#include "random_player.h"
#include "../core/game.h"
#include <chrono>

using namespace std;
using namespace chrono;

threaded_uct_player::threaded_uct_player(int _threads, bool red) :
    uct_player(red),
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

bool threaded_uct_player::think_next_move(pos_move &_move, const board &bd, int8_t no_eat_half_rounds, const vector<pos_move> &)
{
    milliseconds think_time = milliseconds(game::step_time);
    steady_clock::time_point start = steady_clock::now();//steady_clock is best suitable for measuring intervals

    if (!root) {
        root = new threaded_node(game::generate_fen(bd), true, red_side, no_eat_half_rounds);
        node::set_root_depth(root);
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

    _move = best_child->first;
    node* new_root = root->release_child(best_child);
    node::set_root_depth(new_root);
    delete root;
    root = new_root;

    return true;
}

int64_t threaded_uct_player::get_total_simulations() const
{
    return threaded_node::get_total_simulations();
}
