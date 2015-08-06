#include "slow_tree_uct_player.h"
#include "random_player.h"
#include "../core/game.h"
#include <stdexcept>
#include <chrono>
#include <thread>
#include <fstream>
#include <boost/mpi/collectives.hpp>

using namespace std;
using namespace chrono;
namespace mpi = boost::mpi;

slow_tree_uct_player::slow_tree_uct_player(long int sync_period_ms, bool red) :
    root_uct_player(red),
    sync_period(sync_period_ms)
{
    assert(sync_period > 0);
}

bool slow_tree_uct_player::think_next_move(pos_move &_move, const board &bd, uint8_t no_eat_half_rounds, const vector<pos_move> &)
{
    static milliseconds think_time = milliseconds(game::step_time);
    steady_clock::time_point start = steady_clock::now();//steady_clock is best suitable for measuring intervals

    static int world_size = world_comm.size();

    if (!root) {
        root = node::node_ptr(new node(game::generate_fen(bd), red_side, no_eat_half_rounds));
        node::set_root_depth(root);
#ifdef _DEBUG
        cout << "[0] broadcast_tree" << endl;
#endif
        master_send_order(BROADCAST_TREE);
        mpi::broadcast(world_comm, root, 0);
    }

    master_send_order(COMP_LOOP);
    long int synced_point = 0, next_sync_point = sync_period;
    for (milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
         elapsed < think_time;
         elapsed = duration_cast<milliseconds>(steady_clock::now() - start))
    {
        root->select();
        selects++;

        long int current_point = elapsed.count();
        if (synced_point < current_point && current_point >= next_sync_point) {
            master_send_order(SYNC);
            sync_tree();
            synced_point = current_point;
            next_sync_point = synced_point + sync_period;
        }
    }
    master_send_order(COMP_FINISH);
    master_send_order(SYNC);
#ifdef _DEBUG
    start = steady_clock::now();
#endif
    sync_tree();
#ifdef _DEBUG
    cout << "synchronisation after computation took " << duration_cast<milliseconds>(steady_clock::now() - start).count() << " milliseconds" << endl;
#endif

    auto best_child = root->get_best_child();
    if (best_child == root->child_end()) {
        return false;
    }

    _move = (*best_child)->get_move();
    for (int j = 1; j < world_size; ++j) {
        world_comm.send(j, CHILD_SELEC, _move);
    }

    node::node_ptr new_root = root->release_child(best_child);
    node::set_root_depth(new_root);
    root = new_root;

    return true;
}

void slow_tree_uct_player::opponent_moved(const pos_move &m)
{
    if (!root) {
        return;
    }

    for (int i = 1; i < world_comm.size(); ++i) {
        world_comm.send(i, OPP_MOVE, m);
    }

    node::node_ptr new_root;
    auto root_iter = root->find_child(m);
    if (root_iter != root->child_end()) {
        new_root = root->release_child(root_iter);
    } else {
        new_root = root->gen_child_with_a_move(m);
        new_root->set_parent(nullptr);
    }
    root = new_root;
    node::set_root_depth(root);
    master_send_order(SYNC);
    sync_tree();
}

void slow_tree_uct_player::do_slave_job()
{
    bool compute = false;
    do {
        auto probe = world_comm.iprobe(0, mpi::any_tag);
        if (probe) {
            mpi::status status = probe.get();
            if (status.tag() == OPP_MOVE) {
                pos_move m;
                world_comm.recv(0, OPP_MOVE, m);
                node::node_ptr new_root;
                auto root_iter = root->find_child(m);
                if (root_iter != root->child_end()) {
                    new_root = root->release_child(root_iter);
                } else {
                    new_root = root->gen_child_with_a_move(m);
                    new_root->set_parent(nullptr);
                }
                root = new_root;
                node::set_root_depth(root);
            } else if (status.tag() == CHILD_SELEC) {
                pos_move m;
                world_comm.recv(0, CHILD_SELEC, m);
                auto new_root = root->release_child(root->find_child(m));
                root = new_root;
                node::set_root_depth(root);
            } else {
                mpi::status s = world_comm.recv(0, status.tag());
                switch (s.tag()) {
                case SYNC:
                    sync_tree();
                    break;
                case COMP_LOOP:
                    compute = true;
                    break;
                case COMP_FINISH:
                    compute = false;
                    break;
                case BROADCAST_TREE:
                    slave_broadcast_tree();
                    break;
                case REDUCE_SIMS:
                    mpi::reduce(world_comm, selects.load(memory_order_relaxed), plus<uint64_t>(), 0);//std::plus is equivalent to MPI_SUM in C
                    break;
                case EXIT:
                    return;
                default:
                    cerr << "unknown tag: " << s.tag() << endl;
                    throw invalid_argument("received an invalid mpi tag in do_slave_job()");
                }
            }
        } else if (compute) {
            root->select();
            selects++;
        } else {
            static const milliseconds nap(100);
            this_thread::sleep_for(nap);
        }
    } while (true);
}

/*
 * The synchronisation is not performed on the whole tree, hence in our implementation,
 * we just synchronise one-depth (aka root and its children)
 *
 * << Scalability and Parallelization of Monte-Carlo Tree Search >> page 10
 */
void slow_tree_uct_player::sync_tree()
{
    static int world_size = world_comm.size();
    static int rank = world_comm.rank();

#ifdef _DEBUG
    cout << "[" << rank << "] sync_tree" << endl;
#endif

    auto shallow_root = root->make_shallow_copy_with_children();
    shallow_root->set_parent(nullptr);
    vector<node::node_ptr> tree_vec;
    mpi::all_gather(world_comm, shallow_root, tree_vec);
    for(int i = 0; i < world_size; ++i) {
        if (rank == i) {
            continue;
        }
        root->merge(*(tree_vec[i]), true);
    }
}
