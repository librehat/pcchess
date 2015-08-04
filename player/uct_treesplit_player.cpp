#include "uct_treesplit_player.h"
#include "../core/game.h"
#include <forward_list>
#include <chrono>
#include <algorithm>
#include <boost/mpi/collectives.hpp>

using namespace std;
using namespace chrono;
namespace mpi = boost::mpi;

uct_treesplit_player::uct_treesplit_player(int cpu_cores, bool red) :
    root_uct_player(red),
    workers{cpu_cores - 1}
{
    assert(workers > 0);
    thread_vec.resize(workers);
    iq_vec.resize(workers);
}

atomic_bool uct_treesplit_player::stop(false);

bool uct_treesplit_player::think_next_move(pos_move &_move, const board &bd, uint8_t no_eat_half_rounds, const vector<pos_move> &)
{
    if (!root) {
        root = node::node_ptr(new treesplit_node(game::generate_fen(bd), red_side, no_eat_half_rounds));
        master_send_order(BROADCAST_TREE);
        node::set_root_depth(root);
#ifdef _DEBUG
        cout << "[0] broadcast_tree" << endl;
#endif
        mpi::broadcast(world_comm, root, 0);
    }

    master_send_order(TS_START);
    main_thread_start();

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
    thread_safe_queue<treesplit_node::msg_type>().swap(oq);
    for (auto &&q : iq_vec) {
        thread_safe_queue<treesplit_node::msg_type>().swap(q);
    }
    treesplit_node::remove_transmap_useless_entries();
}

void uct_treesplit_player::do_slave_job()
{
    mpi::request request = world_comm.irecv(0, mpi::any_tag);
    do {
        boost::optional<mpi::status> req_ret = request.test();
        if (req_ret) {
            switch (req_ret.get().tag()) {
            case CHILD_SELEC:
                slave_select_child();
                break;
            case BROADCAST_TREE:
                slave_broadcast_tree();
                break;
            case REDUCE_SIMS:
                mpi::reduce(world_comm, node::get_total_simulations(), plus<int>(), 0);//std::plus is equivalent to MPI_SUM in C
                break;
            case TS_START:
                main_thread_start();
                break;
            case TS_BEST_CHILD:
                mpi::gather(world_comm, dynamic_pointer_cast<treesplit_node>(root)->get_best_child_msg(), 0);
                break;
            case EXIT:
                return;
            default:
                cerr << "unknown tag: " << req_ret.get().tag() << endl;
                throw invalid_argument("received an invalid mpi tag in do_slave_job()");
            }
            request = world_comm.irecv(0, mpi::any_tag);
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
    thread_safe_queue<treesplit_node::msg_type>().swap(oq);
    for (auto &&q : iq_vec) {
        thread_safe_queue<treesplit_node::msg_type>().swap(q);
    }
    treesplit_node::remove_transmap_useless_entries();
}

void uct_treesplit_player::main_thread_start()
{
    stop = false;
    for (int i = 0; i < workers; ++i) {
        thread_vec[i] = thread(&uct_treesplit_player::worker_thread, this, i);
    }

    static milliseconds think_time = milliseconds(game::step_time);
    steady_clock::time_point start = steady_clock::now();

    mpi::request ireq = world_comm.irecv(mpi::any_source, TS_MSG);

    for (milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
         elapsed < think_time;
         elapsed = duration_cast<milliseconds>(steady_clock::now() - start))
    {
        if (!oq.empty()) {
            treesplit_node::msg_type omsg = oq.front();
            oq.pop();
            world_comm.send(get<0>(omsg), TS_MSG);
            world_comm.send(get<0>(omsg), TS_MSG_DATA, omsg);
        }

        if (auto s = ireq.test()) {
            treesplit_node::msg_type imsg;
            world_comm.recv(s.get().source(), TS_MSG_DATA, imsg);
            auto min_it = min_element(iq_vec.begin(), iq_vec.end(), [](thread_safe_queue<treesplit_node::msg_type> &x, thread_safe_queue<treesplit_node::msg_type> &y) { return x.size() < y.size(); });
            min_it->push(imsg);
            ireq = world_comm.irecv(mpi::any_source, TS_MSG);
        }
    }
    stop = true;
    //some clean-up
    if (!ireq.test()) {
        ireq.cancel();
    }

    for (auto &&t : thread_vec) {
        t.join();
    }
}

void uct_treesplit_player::worker_thread(const int &iq_id)
{
    treesplit_node::clear_oq();
    do {
        if (!iq_vec[iq_id].empty()) {
            auto imsg = iq_vec[iq_id].front();
            iq_vec[iq_id].pop();
            treesplit_node::insert_node_from_msg(imsg);
        } else {
            root->select();
        }
        if (!treesplit_node::output_queue.empty()) {
            oq.push(treesplit_node::output_queue.front());
            treesplit_node::output_queue.pop();
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
