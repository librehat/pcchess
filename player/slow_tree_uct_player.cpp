#include "slow_tree_uct_player.h"
#include "random_player.h"
#include "../core/game.h"
#include <stdexcept>
#include <chrono>
#include <thread>
#include <fstream>
#include <boost/mpi/collectives.hpp>
#include <boost/serialization/export.hpp>

using namespace std;
using namespace chrono;
namespace mpi = boost::mpi;

BOOST_CLASS_EXPORT_GUID(slow_tree_uct_player, "slow_tree_uct_player")

const int slow_tree_uct_player::TAG_SYNC = 1;
const int slow_tree_uct_player::TAG_OPPMOV = 10;
const int slow_tree_uct_player::TAG_OPPMOV_DATA = 11;
const int slow_tree_uct_player::TAG_CHILD_SELEC = 20;
const int slow_tree_uct_player::TAG_CHILD_SELEC_DATA = 21;
const int slow_tree_uct_player::TAG_COMP_LOOP = 30;
const int slow_tree_uct_player::TAG_COMP_FINISH = 31;
const int slow_tree_uct_player::TAG_BROADCAST_TREE = 40;
const int slow_tree_uct_player::TAG_REDUCE_SIMS = 50;
const int slow_tree_uct_player::TAG_EXIT = 0;

slow_tree_uct_player::slow_tree_uct_player(long int sync_period_ms, bool red) :
    uct_player(red),
    sync_period(sync_period_ms)
{
    assert(sync_period > 0);
    node::set_max_depth(12);
}

slow_tree_uct_player::~slow_tree_uct_player()
{
    if (world_comm.rank() == 0) {
        master_send_order(TAG_EXIT);
    }
}

mpi::communicator slow_tree_uct_player::world_comm;

bool slow_tree_uct_player::think_next_move(pos_move &_move, const board &bd, int8_t no_eat_half_rounds, const vector<pos_move> &banmoves)
{
    static milliseconds think_time = milliseconds(game::step_time);
    steady_clock::time_point start = steady_clock::now();//steady_clock is best suitable for measuring intervals

    static int world_size = world_comm.size();

    if (!root) {
        root = new node(game::generate_fen(bd), true, red_side, no_eat_half_rounds, banmoves);
        master_send_order(TAG_BROADCAST_TREE);
#ifdef _DEBUG
        cout << "[0] broadcast_tree" << endl;
#endif
        mpi::broadcast(world_comm, root, 0);
    }

    master_send_order(TAG_COMP_LOOP);
    long int synced_point = 0, next_sync_point = sync_period;
    bool no_more_sims = false;
    for (milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
         elapsed < think_time;
         elapsed = duration_cast<milliseconds>(steady_clock::now() - start))
    {
        if (no_more_sims) {
            static milliseconds nap(100);
            this_thread::sleep_for(nap);
        } else if (!root->select()) {
            no_more_sims = true;
        }

        long int current_point = elapsed.count();
        if (synced_point < current_point && current_point >= next_sync_point) {
#ifdef _DEBUG
            cout << "sync in the loop, time count: " << current_point << endl;
#endif
            master_send_order(TAG_SYNC);
            sync_tree();
            synced_point = current_point;
            next_sync_point = synced_point + sync_period;
            no_more_sims = false;
        }
    }
    master_send_order(TAG_COMP_FINISH);
    master_send_order(TAG_SYNC);
    sync_tree();

    auto best_child = root->get_best_child();
    if (best_child == root->child_end()) {
        return false;
    }

    _move = best_child->first;
    master_send_order(TAG_CHILD_SELEC);
    for (int i = 1; i < world_size; ++i) {
        world_comm.send(i, TAG_CHILD_SELEC_DATA, _move);
    }

    node* new_root = root->release_child(best_child);
    node::set_root_depth(new_root);
    delete root;
    root = new_root;

    return true;
}

void slow_tree_uct_player::opponent_moved(const pos_move &m)
{
#ifdef _DEBUG
    cout << "opponent_moved" << endl;
#endif

    if (!root) {
        return;
    }

    node *new_root = nullptr;
    auto root_iter = root->find_child(m);
    if (root_iter != root->child_end()) {
        master_send_order(TAG_OPPMOV);
        for (int i = 1; i < world_comm.size(); ++i) {
            world_comm.send(i, TAG_OPPMOV_DATA, m);
        }
        new_root = root->release_child(root_iter);
        node::set_root_depth(new_root);
    }
    delete root;
    root = new_root;
}

void slow_tree_uct_player::master_send_order(const int &tag) const
{
    if (world_comm.rank() != 0) {
        cerr << "non-master node called master_send_order function" << endl;
        return;
    }

    static int world_size = world_comm.size();
    for (int i = 1; i < world_size; ++i) {
        world_comm.send(i, tag);
    }
}

void slow_tree_uct_player::do_slave_job()
{
    mpi::request request = world_comm.irecv(0, mpi::any_tag);
    do {
        boost::optional<mpi::status> req_ret = request.test();
        if (req_ret) {
            switch (req_ret.get().tag()) {
            case TAG_SYNC:
                sync_tree();
                break;
            case TAG_OPPMOV:
                slave_opponent_moved();
                break;
            case TAG_CHILD_SELEC:
                slave_select_child();
                break;
            case TAG_COMP_LOOP:
                slave_compute();
                break;
            case TAG_BROADCAST_TREE:
                slave_broadcast_tree();
                break;
            case TAG_REDUCE_SIMS:
                mpi::reduce(world_comm, node::get_total_simulations(), plus<int>(), 0);//std::plus is equivalent to MPI_SUM in C
                break;
            case TAG_EXIT:
                return;
            default:
                throw invalid_argument("received an invalid mpi tag in do_slave_job()");
            }
            request = world_comm.irecv(0, mpi::any_tag);
        } else {
            static milliseconds nap(100);
            this_thread::sleep_for(nap);
        }
    } while (true);
}

void slow_tree_uct_player::slave_opponent_moved()
{
#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] slave_opponent_moved" << endl;
#endif

    pos_move mov;
    world_comm.recv(0, TAG_OPPMOV_DATA, mov);
    if (!root) {
        throw runtime_error("root is nullptr");
    }

    node *new_root = nullptr;
    auto root_iter = root->find_child(mov);
    if (root_iter != root->child_end()) {
        new_root = root->release_child(root_iter);
        node::set_root_depth(new_root);
    }
    delete root;
    root = new_root;
}

void slow_tree_uct_player::slave_select_child()
{
#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] slave_select_child" << endl;
#endif
    pos_move mov;
    world_comm.recv(0, TAG_CHILD_SELEC_DATA, mov);
    auto root_iter = root->find_child(mov);
    auto new_root = root->release_child(root_iter);
    node::set_root_depth(new_root);
    delete root;
    root = new_root;
}

void slow_tree_uct_player::slave_compute()
{
#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] slave_compute" << endl;
#endif

    if (!root) {
        throw runtime_error("root is nullptr");
    }

    mpi::request request = world_comm.irecv(0, mpi::any_tag);
    bool cont_comp = true, no_more_sims = false;
    do {
        boost::optional<mpi::status> req_ret = request.test();
        if (req_ret) {
            switch (req_ret.get().tag()) {
            case TAG_SYNC:
                sync_tree();
                request = world_comm.irecv(0, mpi::any_tag);//we may get a new order again
                no_more_sims = false;//we may be able to do MCTS again
                break;
            case TAG_COMP_FINISH:
                cont_comp = false;
                break;
            default:
                throw invalid_argument("received an invalid mpi tag in slave_compute()");
            }
        } else {
            if (no_more_sims) {
                static milliseconds nap(100);
                this_thread::sleep_for(nap);
            }
            else if (!root->select()) {
                no_more_sims = true;
            }
        }
    } while (cont_comp);
}

void slow_tree_uct_player::slave_broadcast_tree()
{
#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] broadcast_tree" << endl;
#endif
    if (root) {
        delete root;
        root = nullptr;
    }
    mpi::broadcast(world_comm, root, 0);
    node::set_root_depth(root);
}

void slow_tree_uct_player::sync_tree()
{
    static int world_size = world_comm.size();
    static int rank = world_comm.rank();

#ifdef _DEBUG
    cout << "[" << rank << "] sync_tree" << endl;
#endif

    vector<node*> tree_vec;
#ifdef _DEBUG
    auto start = steady_clock::now();
#endif
    //TODO need to optimise! it takes too long when tree gets bigger
    mpi::all_gather(world_comm, root, tree_vec);
#ifdef _DEBUG
    cout << "[" << rank << "] all_gather took " << duration_cast<milliseconds>(steady_clock::now() - start).count() << " milliseconds" << endl;
#endif

    for(int i = 0; i < world_size; ++i) {
        //don't merge itself
        if (rank == i) {
            continue;
        }
        //merge only if they're different (they might be basically the same if nothing happened between two syncs)
        if (!root->is_basically_the_same(*(tree_vec[i]))) {
            root->merge(*(tree_vec[i]));
        }
    }

    /*
     * Somehow for rank 0, its root is just shallow copied to vector tree_vec,
     * therefore we can't just delete all pointers in tree_vec.
     * instead, delete all pointers that have different addresses than root
     */
    for (auto &&t : tree_vec) {
        if (t != root) {
            delete t;
        }
    }
}

int64_t slow_tree_uct_player::get_total_simulations() const
{
    if (world_comm.rank() != 0) {
        throw runtime_error("non-root's get_total_simulations() gets called");
    }

    master_send_order(TAG_REDUCE_SIMS);
    int64_t local_sims = node::get_total_simulations(), sum_sims = 0;
    mpi::reduce(world_comm, local_sims, sum_sims, plus<int>(), 0);
    return sum_sims;
}
