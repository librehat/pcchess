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
    root_uct_player(red)
{
    if (cpu_cores <= 0) {
        cpu_cores = thread::hardware_concurrency();
    }
    if (cpu_cores == 0) {//thread::hardware_concurrency() information is not available
        cpu_cores = 4;//just a guess that it's a quad-core machine
    }
    workers = cpu_cores - 1;
    thread_vec.resize(workers);
    local_oq_vec.resize(workers);
}

atomic_bool uct_treesplit_player::stop(false);

bool uct_treesplit_player::think_next_move(pos_move &_move, const board &bd, uint8_t no_eat_half_rounds, const vector<pos_move> &)
{
    steady_clock::time_point start = steady_clock::now();
    static milliseconds think_time = milliseconds(game::step_time);

    if (!root) {
        string fen = game::generate_fen(bd);
        master_send_order(TS_INIT);
        mpi::broadcast(world_comm, fen, 0);
        mpi::broadcast(world_comm, no_eat_half_rounds, 0);
        root = node::node_ptr(new treesplit_node(fen, red_side, no_eat_half_rounds, world_comm.rank()));
        node::set_root_depth(root);
    }

    stop = false;
    master_send_order(TS_START);

    for (auto &&oq : local_oq_vec) {
        oq.reset();
    }
    treesplit_node::remove_transmap_useless_entries();
    for (int i = 0; i < workers; ++i) {
        thread_vec[i] = thread(&uct_treesplit_player::worker_thread, this, i);
    }

    for (milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
         elapsed < think_time;
         elapsed = duration_cast<milliseconds>(steady_clock::now() - start))
    {

        auto probe = world_comm.iprobe(mpi::any_source, mpi::any_tag);
        if (probe) {
            mpi::status status = probe.get();
            if (status.tag() == TS_MSG) {
                treesplit_node::msg_type imsg;
                world_comm.recv(status.source(), TS_MSG, imsg);
                treesplit_node::insert_node_from_msg(imsg);
            } else {
                cerr << "unknown tag: " << status.tag() << endl;
                throw invalid_argument("master node received unknown tag");
            }
        }

        auto q = max_element(local_oq_vec.begin(), local_oq_vec.end(), [](lock_free_queue<treesplit_node::msg_type> &x, lock_free_queue<treesplit_node::msg_type> &y){
            return x.size() < y.size();
        });
        if (!q->empty()) {
            treesplit_node::msg_type omsg = q->front();
            q->pop();
            world_comm.send(get<0>(omsg), TS_MSG, omsg);
        }
    }
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
        for (int j = 1; j < world_comm.size(); ++j) {
            world_comm.send(j, CHILD_SELEC, _move);
        }
        evolve_into_next_depth(_move);
    }
    return _move.is_valid();
}

void uct_treesplit_player::opponent_moved(const pos_move &m)
{
    for (int j = 1; j < world_comm.size(); ++j) {
        world_comm.send(j, CHILD_SELEC, m);
    }
    evolve_into_next_depth(m);
}

void uct_treesplit_player::evolve_into_next_depth(const pos_move &m)
{
    auto child_it = root->find_child(m);
    node::node_ptr new_root;
    if (child_it != root->child_end()) {
        new_root = root->release_child(child_it);
    } else {
        new_root = dynamic_pointer_cast<treesplit_node>(root)->generate_root_node_with_move(m);
    }
    root = new_root;
    node::set_root_depth(root);
}

void uct_treesplit_player::do_slave_job()
{
    bool do_io_job = false;
    do {
        auto probe = world_comm.iprobe(mpi::any_source, mpi::any_tag);
        if (probe) {
            mpi::status status = probe.get();
            if (status.tag() == TS_MSG) {
                treesplit_node::msg_type imsg;
                world_comm.recv(status.source(), TS_MSG, imsg);
                treesplit_node::insert_node_from_msg(imsg);
            } else if (status.tag() == CHILD_SELEC) {
                pos_move m;
                world_comm.recv(status.source(), CHILD_SELEC, m);
                evolve_into_next_depth(m);
            } else {
                mpi::status s = world_comm.recv(status.source(), status.tag());
                switch (s.tag()) {
                case TS_INIT:
                    slave_init();
                    break;
                case REDUCE_SIMS:
                    mpi::reduce(world_comm, node::get_total_simulations(), plus<int>(), 0);//std::plus is equivalent to MPI_SUM in C
                    break;
                case TS_START:
                    do_io_job = true;
                    stop = false;
                    for (auto &&oq : local_oq_vec) {
                        oq.reset();
                    }
                    treesplit_node::remove_transmap_useless_entries();
                    for (int i = 0; i < workers; ++i) {
                        thread_vec[i] = thread(&uct_treesplit_player::worker_thread, this, i);
                    }
                    break;
                case TS_STOP:
                    do_io_job = false;
                    stop = true;
                    for (auto &&t : thread_vec) {
                        t.join();
                    }
                    break;
                case TS_BEST_CHILD:
                    mpi::gather(world_comm, dynamic_pointer_cast<treesplit_node>(root)->get_best_child_msg(), 0);
                    break;
                case EXIT:
                    return;
                default:
                    cerr << "unknown tag: " << s.tag() << endl;
                    throw invalid_argument("received an invalid mpi tag in do_slave_job()");
                }
            }
        } else if (do_io_job) {//IO job
            auto q = max_element(local_oq_vec.begin(), local_oq_vec.end(), [](lock_free_queue<treesplit_node::msg_type> &x, lock_free_queue<treesplit_node::msg_type> &y){
                return x.size() < y.size();
            });
            if (!q->empty()) {
                treesplit_node::msg_type omsg = q->front();
                q->pop();
                world_comm.send(get<0>(omsg), TS_MSG, omsg);
            }
        } else {
            static const milliseconds nap(100);
            this_thread::sleep_for(nap);
        }
    } while (true);
}

void uct_treesplit_player::slave_init()
{
    string fen;
    uint8_t no_eat_half_rounds;
    mpi::broadcast(world_comm, fen, 0);
    mpi::broadcast(world_comm, no_eat_half_rounds, 0);
    root = node::node_ptr(new treesplit_node(fen, red_side, no_eat_half_rounds, world_comm.rank()));
    node::set_root_depth(root);
}

void uct_treesplit_player::worker_thread(const int &id)
{
    treesplit_node::clear_output_queue();
    do {
        root->select();
        while (!treesplit_node::output_queue.empty()) {
            local_oq_vec[id].push(treesplit_node::output_queue.front());
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
