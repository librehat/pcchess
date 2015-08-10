#include "root_uct_player.h"
#include "../core/game.h"
#include <chrono>
#include <thread>
#include <boost/mpi/collectives.hpp>

using namespace std;
using namespace chrono;
namespace mpi = boost::mpi;

root_uct_player::root_uct_player(bool red) :
    uct_player(red)
{
    node::set_max_depth(6);//this is a moderate depth for root_uct_player. bigger number results in too long whole-tree synchronisation
}

root_uct_player::~root_uct_player()
{
    if (world_comm.rank() == 0) {
        master_send_order(EXIT);
    }
#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] root_uct_player destructor called" << endl;
#endif
}

mpi::communicator root_uct_player::world_comm;

bool root_uct_player::think_next_move(pos_move &_move, const board &bd, uint8_t no_eat_half_rounds, const vector<pos_move> &)
{
    static milliseconds think_time = milliseconds(game::step_time);
    steady_clock::time_point start = steady_clock::now();//steady_clock is best suitable for measuring intervals

    master_send_order(BROADCAST_TREE);
    if (!root) {
        root = node::node_ptr(new node(game::generate_fen(bd), red_side, no_eat_half_rounds));
        node::set_root_depth(root);
        mpi::broadcast(world_comm, root, 0);
    } else {
        node::node_ptr shallow_root = root->make_shallow_copy();
        shallow_root->set_parent(nullptr);
        mpi::broadcast(world_comm, shallow_root, 0);
    }
#ifdef _DEBUG
    cout << "broadcasting tree took " << duration_cast<milliseconds>(steady_clock::now() - start).count() << " milliseconds" << endl;
#endif

    master_send_order(COMP_LOOP);
    for (milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
         elapsed < think_time;
         elapsed = duration_cast<milliseconds>(steady_clock::now() - start))
    {
        root->select();
        selects++;
    }
    master_send_order(COMP_FINISH);

#ifdef _DEBUG
    start = steady_clock::now();
#endif
    master_send_order(GATHER_TREE);
    vector<node::node_ptr> root_vec;
    mpi::gather(world_comm, root, root_vec, 0);
#ifdef _DEBUG
    cout << "gathering tree took " << duration_cast<milliseconds>(steady_clock::now() - start).count() << " milliseconds" << endl;
    start = steady_clock::now();
#endif
    for (int i = 1; i < world_comm.size(); ++i) {//skip myself
        root->merge(*(root_vec[i]));
    }
#ifdef _DEBUG
    cout << "merging tree took " << duration_cast<milliseconds>(steady_clock::now() - start).count() << " milliseconds" << endl;
#endif

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

void root_uct_player::do_slave_job()
{
    bool compute = false;
    do {
        auto probe = world_comm.iprobe(0, mpi::any_tag);
        if (probe) {
            mpi::status status = probe.get();
            world_comm.recv(0, status.tag());
            switch (status.tag()) {
            case GATHER_TREE:
                mpi::gather(world_comm, root, 0);
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
                slave_reduce_sims();
                break;
            case EXIT:
                return;
            default:
                throw invalid_argument("received an invalid mpi tag in do_slave_job()");
            }
        } else if (compute) {
            root->select();
            selects++;
        } else {
            static const milliseconds nap(50);
            this_thread::sleep_for(nap);
        }
    } while (true);
}

void root_uct_player::master_send_order(const TAG &tag) const
{
    if (world_comm.rank() != 0) {
        cerr << "non-master node called master_send_order function" << endl;
        return;
    }

    static const int world_size = world_comm.size();
    for (int i = 1; i < world_size; ++i) {
        world_comm.send(i, tag);
    }
}

void root_uct_player::slave_broadcast_tree()
{
#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] broadcast_tree" << endl;
#endif
    if (root) {
        root.reset();
    }
    mpi::broadcast(world_comm, root, 0);
    node::set_root_depth(root);
}

void root_uct_player::slave_reduce_sims()
{
    uint64_t local_sims = selects.load(memory_order_relaxed);
    mpi::reduce(world_comm, local_sims, plus<uint64_t>(), 0);//std::plus is equivalent to MPI_SUM in C
}

uint64_t root_uct_player::get_total_simulations() const
{
    if (world_comm.rank() != 0) {
        throw runtime_error("non-root's get_total_simulations() gets called");
    }

    master_send_order(REDUCE_SIMS);
    uint64_t sum_sims = 0, local_sims = selects.load(memory_order_relaxed);
    mpi::reduce(world_comm, local_sims, sum_sims, plus<uint64_t>(), 0);
    return sum_sims;
}
