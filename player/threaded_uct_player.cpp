#include "threaded_uct_player.h"
#include "threaded_node.h"
#include "random_player.h"
#include "../core/game.h"
#include <chrono>

using namespace std;
using namespace chrono;

threaded_uct_player::threaded_uct_player(int threads, bool red) :
    uct_player(red),
    stop(false)
{
    if (threads <= 0) {
        threads = thread::hardware_concurrency();
    }
    if (threads == 0) {
        threads = 4;//assume it's a quad-core machine then
    }
    thread_vec.resize(threads - 1);
}

bool threaded_uct_player::think_next_move(pos_move &_move, const board &bd, uint8_t no_eat_half_rounds, const vector<pos_move> &)
{
    milliseconds think_time = milliseconds(game::step_time);
    steady_clock::time_point start = steady_clock::now();//steady_clock is best suitable for measuring intervals

    if (!root) {
        root = node::node_ptr(new threaded_node(game::generate_fen(bd), red_side, no_eat_half_rounds));
        node::set_root_depth(root);
    }

    stop = false;
    for (auto &t : thread_vec) {
        t = thread(&threaded_uct_player::worker_thread, this);
    }
    for (milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
         elapsed < think_time;
         elapsed = duration_cast<milliseconds>(steady_clock::now() - start))
    {
        root->select();
        selects++;
    }
    stop = true;
    for (auto &t : thread_vec) {
        t.join();
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

void threaded_uct_player::worker_thread()
{
    do {
        root->select();
        selects++;
    } while (!stop.load(std::memory_order_relaxed));
}
