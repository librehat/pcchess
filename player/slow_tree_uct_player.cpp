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

slow_tree_uct_player::slow_tree_uct_player(long int sync_period_ms, bool opposite) :
    uct_player(opposite),
    sync_period(sync_period_ms)
{
    assert(sync_period > 0);
}

mpi::communicator slow_tree_uct_player::world_comm;

bool slow_tree_uct_player::think_next_move(pos_move &_move, const board &, const abstract_player &opponent, unsigned int no_eat_half_rounds, const vector<pos_move> &banmoves)
{
    milliseconds think_time = milliseconds(game::step_time);
    steady_clock::time_point start = steady_clock::now();//steady_clock is best suitable for measuring intervals

    static int world_size = world_comm.size();

    if (!root) {
        root = new node(new random_player(*this), new random_player(opponent), true, no_eat_half_rounds, banmoves);
        broadcast_tree();
    }

    vector<mpi::request> request_vec;
    for (int i = 1; i < world_size; ++i) {
        request_vec.push_back(world_comm.isend(i, TAG_COMP_LOOP));
    }
    for (auto &&rt : request_vec) {
        rt.wait();
    }
    request_vec.clear();

    long int synced_point = 0, next_sync_point = sync_period;
    bool no_more_sims = false;
    for (milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
         elapsed < think_time;
         elapsed = duration_cast<milliseconds>(steady_clock::now() - start))
    {
        long int current_point = elapsed.count();
        if (synced_point < current_point && current_point >= next_sync_point) {
            for (int i = 1; i < world_size; ++i) {
                world_comm.send(i, TAG_SYNC);
            }
#ifdef _DEBUG
            cout << "sync in the loop, time count: " << current_point << endl;
#endif
            sync_tree();
            synced_point = current_point;
            next_sync_point = synced_point + sync_period;
            no_more_sims = false;
        }
        if (no_more_sims) {
            static milliseconds nap(100);
            this_thread::sleep_for(nap);
        } else if (!root->select()) {
            no_more_sims = true;
        }
    }
    for (int i = 1; i < world_size; ++i) {
        world_comm.send(i, TAG_COMP_FINISH);
    }

    for (int i = 1; i < world_size; ++i) {
        world_comm.send(i, TAG_SYNC);
    }
    sync_tree();

    auto best_child = root->get_best_child();
    if (best_child == root->child_end()) {
        for (int i = 1; i < world_size; ++i) {
            world_comm.send(i, TAG_ERASE);
        }
        return false;
    }

    node* new_root = root->release_child(best_child);
    for (int i = 1; i < world_size; ++i) {
        request_vec.push_back(world_comm.isend(i, TAG_CHILD_SELEC));
    }
    for (int i = 1; i < world_size; ++i) {
        request_vec[i - 1].wait();
        request_vec[i - 1] = world_comm.isend(i, TAG_CHILD_SELEC_DATA, new_root->get_our_move());
    }

    delete root;
    root = new_root;
    _move = root->get_our_move();

    for (auto &&rt : request_vec) {
        rt.wait();
    }

    return true;
}

void slow_tree_uct_player::opponent_moved(const pos_move &m, const abstract_player &opponent, unsigned int no_eat_half_rounds)
{
    if (!root) {
        return;
    }

    node *new_root = nullptr;
    auto root_iter = root->find_child(m);
    if (root_iter != root->child_end()) {
        for (int i = 1; i < world_comm.size(); ++i) {
            world_comm.send(i, TAG_OPPMOV);
        }
        for (int i = 1; i < world_comm.size(); ++i) {
            world_comm.send(i, TAG_OPPMOV_DATA, m);
        }
        new_root = root->release_child(root_iter);
        delete root;
        root = new_root;
    } else {
        delete root;
        root = new node(new random_player(*this), new random_player(opponent), true, no_eat_half_rounds);
        broadcast_tree();
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
                broadcast_tree();
                break;
            case TAG_ERASE:
                delete root;
                root = nullptr;
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
        return;
    }

    node *new_root = nullptr;
    auto root_iter = root->find_child(mov);
    if (root_iter != root->child_end()) {
        new_root = root->release_child(root_iter);
    }
    delete root;
    root = new_root;
}

void slow_tree_uct_player::slave_select_child()
{
    pos_move mov;
    world_comm.recv(0, TAG_CHILD_SELEC_DATA, mov);
    auto root_iter = root->find_child(mov);
    auto new_root = root->release_child(root_iter);
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
        if (no_more_sims) {
            static milliseconds nap(100);
            this_thread::sleep_for(nap);
        }
        else if (!root->select()) {
            no_more_sims = true;
        }
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
        }
    } while (cont_comp);
}

void slow_tree_uct_player::broadcast_tree()
{
#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] broadcast_tree" << endl;
#endif

    if (world_comm.rank() == 0) {
        for (int i = 1; i < world_comm.size(); ++i) {
            world_comm.send(i, TAG_BROADCAST_TREE);
        }
    } else {
        if (root) {
            delete root;
            root = nullptr;
        }
    }
    mpi::broadcast(world_comm, root, 0);
}

void slow_tree_uct_player::sync_tree()
{
#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] children_size (before sync): " << root->children_size() << endl;
#endif

    static int world_size = world_comm.size();
    vector<node*> tree_vec;

    mpi::all_gather(world_comm, root, tree_vec);

    int rank = world_comm.rank();
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

#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] children_size (after sync): " << root->children_size() << endl;
#endif
}
