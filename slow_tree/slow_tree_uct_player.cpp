#include "slow_tree_uct_player.h"
#include "../core/random_player.h"
#include "../core/game.h"
#include "../core/serialization_export.h"
#include <stdexcept>
#include <chrono>

using namespace std;
using namespace chrono;
namespace mpi = boost::mpi;

BOOST_CLASS_EXPORT_GUID(slow_tree_uct_player, "slow_tree_uct_player")

slow_tree_uct_player::slow_tree_uct_player(const abstract_player * const _opp, bool opposite) :
    uct_player(_opp, opposite)
{}

mpi::communicator slow_tree_uct_player::world_comm;

bool slow_tree_uct_player::think_next_move(pos_move &_move, const board &)
{
    if (!root) {
        root = new node(new random_player(*this), new random_player(*opponent), true);
        broadcast_tree();
    }

    vector<mpi::request> request_vec;
    for (int i = 1; i < world_comm.size(); ++i) {
        request_vec.push_back(world_comm.isend(i, TAG_COMP));
    }

    duration<double> think_time = duration<double>(game::step_time);
    time_point<steady_clock> start = steady_clock::now();//steady_clock is best suitable for measuring intervals
    for (duration<double> elapsed = steady_clock::now() - start;
         elapsed < think_time;
         elapsed = steady_clock::now() - start)
    {
        if (!root->select()) {
            break;
        }
    }

    for (auto &&rt : request_vec) {
        rt.wait();
    }
    request_vec.clear();
    sync_tree();

    auto best_child = root->get_best_child();
    if (best_child == root->child_end()) {
        for (int i = 1; i < world_comm.size(); ++i) {
            world_comm.send(i, TAG_ERASE);
        }
        return false;
    }

    node* new_root = root->release_child(best_child);
    for (int i = 1; i < world_comm.size(); ++i) {
        request_vec.push_back(world_comm.isend(i, TAG_CHILD_SELEC));
    }
    for (int i = 1; i < world_comm.size(); ++i) {
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

void slow_tree_uct_player::opponent_moved(const pos_move &m)
{
    vector<mpi::request> request_vec;
    for (int i = 1; i < world_comm.size(); ++i) {
        request_vec.push_back(world_comm.isend(i, TAG_OPPMOV));
    }
    for (int i = 1; i < world_comm.size(); ++i) {
        request_vec[i - 1].wait();
        request_vec[i - 1] = world_comm.isend(i, TAG_OPPMOV_DATA, m);
    }

    /* then we do a similar job as uct_player does */
    if (!root) {
        return;
    }

    node *new_root = nullptr;
    auto root_iter = root->find_child(m);
    for (auto &&rt : request_vec) {
        rt.wait();
    }
    if (root_iter != root->child_end()) {
        new_root = root->release_child(root_iter);
        delete root;
        root = new_root;
    } else {
        delete root;
        root = new node(new random_player(*this), new random_player(*opponent), true);
        broadcast_tree();
    }
}

void slow_tree_uct_player::do_slave_job()
{
    int tag;
    mpi::status status;

    do {
        if (!root) {
            broadcast_tree();
        }

        status = world_comm.recv(0, mpi::any_tag);
        tag = status.tag();

        switch (tag) {
        case TAG_SYNC:
            sync_tree();
            break;
        case TAG_OPPMOV:
            slave_opponent_moved();
            break;
        case TAG_CHILD_SELEC:
            slave_select_child();
            break;
        case TAG_COMP:
            slave_compute();
            break;
        case TAG_ERASE:
            delete root;
            root = nullptr;
            break;
        case TAG_EXIT:
            return;
        default:
            throw invalid_argument("Unknown tag");
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
    root = new_root;//if it's nullptr, the slave loop would let it go into broadcast_tree
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

    duration<double> think_time = duration<double>(game::step_time);
    time_point<steady_clock> start = steady_clock::now();
    for (duration<double> elapsed = steady_clock::now() - start;
         elapsed < think_time;
         elapsed = steady_clock::now() - start)
    {
        if (!root->select()) {
            break;
        }
    }
    sync_tree();
}

void slow_tree_uct_player::broadcast_tree()
{
#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] broadcast_tree" << endl;
#endif

    mpi::broadcast(world_comm, root, 0);
}

void slow_tree_uct_player::sync_tree()
{
#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] children_size (before sync): " << root->children_size() << endl;
#endif

    int size = world_comm.size();
    vector<node*> tree_vec;//TODO is there a memory leak?

    mpi::all_gather(world_comm, root, tree_vec);

    int rank = world_comm.rank();
    for(int i = 0; i < size; ++i) {
        if (rank == i) {//don't merge itself
            continue;
        }
        root->merge(*(tree_vec[i]));
    }

#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] children_size (after sync): " << root->children_size() << endl;
#endif
}
