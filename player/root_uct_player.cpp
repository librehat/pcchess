#include "root_uct_player.h"
#include "../core/game.h"
#include <chrono>
#include <thread>
#include <boost/mpi/collectives.hpp>
#include <boost/serialization/export.hpp>

using namespace std;
using namespace chrono;
namespace mpi = boost::mpi;

BOOST_CLASS_EXPORT_GUID(root_uct_player, "root_uct_player")

root_uct_player::root_uct_player(bool red) :
    uct_player(red)
{
    node::set_max_depth(12);
}

root_uct_player::~root_uct_player()
{
    if (world_comm.rank() == 0) {
        master_send_order(EXIT);
    }
}

mpi::communicator root_uct_player::world_comm;

bool root_uct_player::think_next_move(pos_move &_move, const board &bd, int8_t no_eat_half_rounds, const vector<pos_move> &)
{
    static milliseconds think_time = milliseconds(game::step_time);
    steady_clock::time_point start = steady_clock::now();//steady_clock is best suitable for measuring intervals

    if (!root) {
        root = new node(game::generate_fen(bd), true, red_side, no_eat_half_rounds);
    }
    master_send_order(BROADCAST_TREE);
    mpi::broadcast(world_comm, root, 0);
#ifdef _DEBUG
    cout << "broadcasting tree took " << duration_cast<milliseconds>(steady_clock::now() - start).count() << " milliseconds" << endl;
#endif

    master_send_order(COMP_LOOP);
    for (milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
         elapsed < think_time;
         elapsed = duration_cast<milliseconds>(steady_clock::now() - start))
    {
        if (!root->select()) {
            break;
        }
    }
    master_send_order(COMP_FINISH);

#ifdef _DEBUG
    start = steady_clock::now();
#endif
    master_send_order(GATHER_TREE);
    vector<node*> root_vec;
    mpi::gather(world_comm, root, root_vec, 0);
    for (int i = 1; i < world_comm.size(); ++i) {//skip myself
        root->merge(*(root_vec[i]));
    }
    for (auto &&i : root_vec) {
        if (i != root) {
            delete i;
        }
    }
#ifdef _DEBUG
    cout << "gathering and merging tree took " << duration_cast<milliseconds>(steady_clock::now() - start).count() << " milliseconds" << endl;
#endif

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

void root_uct_player::do_slave_job()
{
    mpi::request request = world_comm.irecv(0, mpi::any_tag);
    do {
        boost::optional<mpi::status> req_ret = request.test();
        if (req_ret) {
            switch (req_ret.get().tag()) {
            case GATHER_TREE:
                mpi::gather(world_comm, root, 0);
                break;
            case COMP_LOOP:
                slave_compute();
                break;
            case BROADCAST_TREE:
                slave_broadcast_tree();
                break;
            case REDUCE_SIMS:
                mpi::reduce(world_comm, node::get_total_simulations(), plus<int>(), 0);//std::plus is equivalent to MPI_SUM in C
                break;
            case EXIT:
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

void root_uct_player::master_send_order(const TAG &tag) const
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

void root_uct_player::slave_compute()
{
    mpi::request request = world_comm.irecv(0, mpi::any_tag);
    do {
        auto test = request.test();
        if (test) {
            if (test.get().tag() == COMP_FINISH) {
                return;
            } else {
                throw runtime_error("slave_compute received an invalid tag");
            }
        } else {
            if (!root->select()) {
                static milliseconds nap(100);
                this_thread::sleep_for(nap);
            }
        }
    } while (true);
}

void root_uct_player::slave_broadcast_tree()
{
    if (root) {
        delete root;
        root = nullptr;
    }
    mpi::broadcast(world_comm, root, 0);
    node::set_root_depth(root);
}

int64_t root_uct_player::get_total_simulations() const
{
    if (world_comm.rank() != 0) {
        throw runtime_error("non-root's get_total_simulations() gets called");
    }

    master_send_order(REDUCE_SIMS);
    int64_t local_sims = node::get_total_simulations(), sum_sims = 0;
    mpi::reduce(world_comm, local_sims, sum_sims, plus<int>(), 0);
    return sum_sims;
}
