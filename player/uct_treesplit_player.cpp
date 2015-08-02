#include "uct_treesplit_player.h"
#include "../core/game.h"
#include <forward_list>
#include <chrono>
#include <algorithm>
#include <boost/mpi/collectives.hpp>

using namespace std;
using namespace chrono;
namespace mpi = boost::mpi;

uct_treesplit_player::uct_treesplit_player(int cores, bool red) :
    root_uct_player(red),
    cpu_cores{cores - 1}
{
    assert(cpu_cores > 1);
    thread_vec.resize(cpu_cores);
    iq_vec.resize(cpu_cores - 1);
}

atomic_bool uct_treesplit_player::stop(false);

bool uct_treesplit_player::think_next_move(pos_move &_move, const board &bd, uint8_t no_eat_half_rounds, const vector<pos_move> &)
{
    static milliseconds think_time = milliseconds(game::step_time);
    steady_clock::time_point start = steady_clock::now();

    if (!root) {
        root = node::node_ptr(new treesplit_node(game::generate_fen(bd), red_side, no_eat_half_rounds));
        master_send_order(BROADCAST_TREE);
        node::set_root_depth(root);
        mpi::broadcast(world_comm, root, 0);
#ifdef _DEBUG
        cout << "broadcasting tree took " << duration_cast<milliseconds>(steady_clock::now() - start).count() << " milliseconds" << endl;
#endif
    }

    master_send_order(TS_START);
    thread_vec[0] = thread(&uct_treesplit_player::io_thread, this);
    for (int i = 1; i < cpu_cores; ++i) {
        thread_vec[i] = thread(&uct_treesplit_player::worker_thread, this, i - 1);
    }
    this_thread::sleep_for(think_time - duration_cast<milliseconds>(steady_clock::now() - start));
    master_send_order(TS_STOP);
    stop = true;
    for (auto &&t : thread_vec) {
        t.join();
    }

    master_send_order(TS_BEST_CHILD);
    vector<treesplit_node::child_type> bchild_vec;
    mpi::gather(world_comm, dynamic_pointer_cast<treesplit_node>(root)->get_best_child_msg(), bchild_vec, 0);
    auto best_child = max_element(bchild_vec.begin(), bchild_vec.end(), [](const treesplit_node::child_type &x, const treesplit_node::child_type &y) { return get<1>(x) < get<1>(y); });

    _move = get<0>(*best_child);
    if (_move.is_valid()) {
        evolve_into_next_depth(_move);
    }
    return _move.is_valid();
}

void uct_treesplit_player::opponent_moved(const pos_move &m)
{
    evolve_into_next_depth(m);
}

void uct_treesplit_player::evolve_into_next_depth(const pos_move &m)
{
    assert (world_comm.rank() == 0);

    master_send_order(CHILD_SELEC);
    for (int j = 1; j < world_comm.size(); ++j) {
        world_comm.send(j, CHILD_SELEC_DATA, m);
    }
    auto child_it = root->find_child(m);
    node::node_ptr new_root;
    if (child_it != root->child_end()) {
        new_root = root->release_child(child_it);
    } else {
        new_root = root->gen_child_with_a_move(m);
        new_root->set_parent(nullptr);
    }
    root = new_root;
    node::set_root_depth(root);
    treesplit_node::clean_queue_map();
    for (auto &&q : iq_vec) {
        thread_safe_queue<treesplit_node::msg_type>().swap(q);
    }
}

void uct_treesplit_player::do_slave_job()
{
    mpi::request exit_req = world_comm.irecv(0, EXIT);
    mpi::request bcast_req = world_comm.irecv(0, BROADCAST_TREE);
    mpi::request rsims_req = world_comm.irecv(0, REDUCE_SIMS);
    mpi::request start_req = world_comm.irecv(0, TS_START);
    mpi::request stop_req = world_comm.irecv(0, TS_STOP);
    mpi::request bchild_req = world_comm.irecv(0, TS_BEST_CHILD);
    mpi::request selec_req = world_comm.irecv(0, CHILD_SELEC);

    do {
        if (start_req.test()) {
            thread_vec[0] = thread(&uct_treesplit_player::io_thread, this);
            for (int i = 1; i < cpu_cores; ++i) {
                thread_vec[i] = thread(&uct_treesplit_player::worker_thread, this, i - 1);
            }
            start_req = world_comm.irecv(0, TS_START);
        } else if (stop_req.test()) {
            stop = true;
            for (auto &&t : thread_vec) {
                t.join();
            }
            stop_req = world_comm.irecv(0, TS_STOP);
        } else if (bcast_req.test()) {
            slave_broadcast_tree();
            bcast_req = world_comm.irecv(0, BROADCAST_TREE);
        } else if (rsims_req.test()) {
            mpi::reduce(world_comm, treesplit_node::get_total_simulations(), plus<int>(), 0);//std::plus is equivalent to MPI_SUM in C
            rsims_req = world_comm.irecv(0, REDUCE_SIMS);
        } else if (exit_req.test()) {
            return;
        } else if (bchild_req.test()) {
            mpi::gather(world_comm, dynamic_pointer_cast<treesplit_node>(root)->get_best_child_msg(), 0);
            bchild_req = world_comm.irecv(0, TS_BEST_CHILD);
        } else if (selec_req.test()) {
            slave_select_child();
            selec_req = world_comm.irecv(0, CHILD_SELEC);
        } else {
            static milliseconds nap(100);
            this_thread::sleep_for(nap);
        }
    } while (true);
}

void uct_treesplit_player::slave_select_child()
{
    pos_move m;
    world_comm.recv(0, CHILD_SELEC_DATA, m);
    auto child_it = root->find_child(m);
    node::node_ptr new_root;
    if (child_it != root->child_end()) {
        new_root = root->release_child(child_it);
    } else {
        new_root = root->gen_child_with_a_move(m);
        new_root->set_parent(nullptr);
    }
    root = new_root;
    node::set_root_depth(root);
    treesplit_node::clean_queue_map();
    for (auto &&q : iq_vec) {
        thread_safe_queue<treesplit_node::msg_type>().swap(q);
    }
}

void uct_treesplit_player::io_thread()
{
#ifdef _DEBUG
    cout << "io thread" << endl;
#endif
    treesplit_node::msg_type imsg;
    mpi::request ireq = world_comm.irecv(mpi::any_source, TS_MSG, imsg);
    forward_list<mpi::request> oreq_list;

    do {
        if (!treesplit_node::output_queue.empty()) {//TODO make output_queue local
            treesplit_node::msg_type omsg = treesplit_node::output_queue.front();
            treesplit_node::output_queue.pop();
            oreq_list.push_front(world_comm.isend(get<0>(omsg), TS_MSG, omsg));
        }
        oreq_list.remove_if([](mpi::request &r) { return r.test(); });//remove finished requests

        if (ireq.test()) {
            size_t min_size = iq_vec.front().size();
            auto iq_it = iq_vec.begin();
            auto min_it = iq_it;
            for (++iq_it; iq_it != iq_vec.end(); ++iq_it) {
                size_t size = iq_it->size();
                if (size < min_size) {
                    min_size = size;
                    min_it = iq_it;
                }
            }
            min_it->push(imsg);
            ireq = world_comm.irecv(mpi::any_source, TS_MSG, imsg);
        }
    } while (!stop);

    //some clean-up
    if (!ireq.test()) {
        ireq.cancel();
    }
    oreq_list.remove_if([](mpi::request &r) { return r.test(); });
    for (auto oit = oreq_list.begin(); oit != oreq_list.end(); ++oit) {
        oit->cancel();
    }
}

void uct_treesplit_player::worker_thread(int iq_id)
{
#ifdef _DEBUG
    cout << "worker thread" << endl;
#endif
    do {
        if (!iq_vec[iq_id].empty()) {
            auto imsg = iq_vec[iq_id].front();
            iq_vec[iq_id].pop();
            treesplit_node::insert_node_from_msg(imsg);
        } else {
            root->select();
        }
    } while (!stop);
}

int64_t uct_treesplit_player::get_total_simulations() const
{
    if (world_comm.rank() != 0) {
        throw runtime_error("non-root's get_total_simulations() gets called");
    }

    master_send_order(REDUCE_SIMS);
    int64_t local_sims = treesplit_node::get_total_simulations(), sum_sims = 0;
    mpi::reduce(world_comm, local_sims, sum_sims, plus<int>(), 0);
    return sum_sims;
}
