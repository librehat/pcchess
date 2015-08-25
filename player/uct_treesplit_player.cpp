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
    stop(false)
{
    if (cpu_cores <= 0) {
        cpu_cores = thread::hardware_concurrency();
    }
    if (cpu_cores == 0) {//thread::hardware_concurrency() information is not available
        cpu_cores = 4;//just a guess that it's a quad-core machine
    }
    workers = cpu_cores - 1;
    thread_vec.resize(workers);
    local_iq_vec.resize(workers);
    local_oq_vec.resize(workers);
}

bool uct_treesplit_player::think_next_move(pos_move &_move, const board &bd, uint8_t no_eat_half_rounds, const vector<pos_move> &)
{
    steady_clock::time_point start = steady_clock::now();
    static milliseconds think_time = milliseconds(game::step_time);

    if (!root) {
        root = node::node_ptr(new treesplit_node(game::generate_fen(bd), red_side, no_eat_half_rounds, world_comm.rank()));
        node::set_root_depth(root);
        master_send_order(BROADCAST_TREE);
        mpi::broadcast(world_comm, root, 0);
    }

    stop = false;
    master_send_order(TS_START);

    for (int i = 0; i < workers; ++i) {
        local_oq_vec[i].reset();
        local_iq_vec[i].reset();
    }
#ifdef UCT_TREESPLIT_CLEAN_BEFORE_CALC
    /*
     * this function uses too much time. as a workaround, we use a relatively huge transposition table,
     * and clear it (which should be an optimised O(n) operation) after the each game by default
     */
    treesplit_node::remove_transmap_useless_entries();
#endif
    for (int i = 0; i < workers; ++i) {
        thread_vec[i] = thread(&uct_treesplit_player::worker_thread, this, i);
    }

    static const int world_size = world_comm.size();
    for (milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
         elapsed < think_time;
         elapsed = duration_cast<milliseconds>(steady_clock::now() - start))
    {
        auto probe = world_comm.iprobe(mpi::any_source, mpi::any_tag);
        if (probe) {
            mpi::status status = probe.get();
            if (status.tag() == TS_MSG) {
#ifdef _DEBUG
                cout << "[" << world_comm.rank() << "] received a treesplit message" << endl;
#endif
                treesplit_node::msg_type imsg;
                world_comm.recv(status.source(), TS_MSG, imsg);
                auto q = min_element(local_iq_vec.begin(), local_iq_vec.end(), [](const lf_queue &x, const lf_queue &y){
                    return x.size() < y.size();
                });
                q->push(imsg);
#ifdef _DEBUG
                cout << "[" << world_comm.rank() << "] received treesplit message rank: " << get<0>(imsg) << endl;
#endif
            } else {
                cerr << "unknown tag: " << status.tag() << endl;
                throw invalid_argument("master node received unknown tag");
            }
        }

        auto q = max_element(local_oq_vec.begin(), local_oq_vec.end(), [](const lf_queue &x, const lf_queue &y){
            return x.size() < y.size();
        });
        if (!q->empty()) {
            auto omsg = q->front();
            q->pop();
            if (get<0>(omsg) == -1) {
#ifdef _DEBUG
                cout << "[" << world_comm.rank() << "] send out a duplicate message" << endl;
#endif
                for (int i = 1; i < world_size; ++i) {//i'm the rank 0
                    pending_requests.push_back(world_comm.isend(i, TS_MSG, omsg));
                }
            } else {
#ifdef _DEBUG
                cout << "[" << world_comm.rank() << "] send out an update message" << endl;
#endif
                pending_requests.push_back(world_comm.isend(get<0>(omsg), TS_MSG, omsg));
            }
        }
    }
    master_send_order(TS_STOP);
    stop = true;
    for (auto &&t : thread_vec) {
        t.join();
    }
    for (auto &&req : pending_requests) {
        if (!req.test()) {
            req.cancel();
        }
    }
    pending_requests.clear();

    master_send_order(TS_BEST_CHILD);
    vector<treesplit_node::child_type> bchild_vec;
#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] gathering best children" << endl;
#endif
    mpi::gather(world_comm, dynamic_pointer_cast<treesplit_node>(root)->get_best_child_msg(), bchild_vec, 0);
    auto best_child = max_element(bchild_vec.begin(), bchild_vec.end(), [](const treesplit_node::child_type &x, const treesplit_node::child_type &y) { return get<1>(x) < get<1>(y); });

    _move = get<0>(*best_child);
    if (_move.is_valid()) {
        opponent_moved(_move);
    }
    return _move.is_valid();
}

void uct_treesplit_player::opponent_moved(const pos_move &m)
{
    static int world_size = world_comm.size();
    for (int j = 1; j < world_size; ++j) {
        pending_requests.push_back(world_comm.isend(j, CHILD_SELEC, m));
    }
    for (auto &&req : pending_requests) {
        if (!req.test()) {
            req.wait();
        }
    }
    pending_requests.clear();
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
    static const int rank = world_comm.rank();
    static const int world_size = world_comm.size();
    do {
        auto probe = world_comm.iprobe(mpi::any_source, mpi::any_tag);
        if (probe) {
            mpi::status status = probe.get();
            if (status.tag() == TS_MSG) {
#ifdef _DEBUG
                cout << "[" << rank << "] received a treesplit message" << endl;
#endif
                treesplit_node::msg_type imsg;
                world_comm.recv(status.source(), TS_MSG, imsg);
                min_element(local_iq_vec.begin(), local_iq_vec.end(), [](const lf_queue &x, const lf_queue &y){
                    return x.size() < y.size();
                })->push(imsg);
#ifdef _DEBUG
                cout << "[" << rank << "] received treesplit message rank: " << get<0>(imsg) << endl;
#endif
            } else if (status.tag() == CHILD_SELEC) {
                pos_move m;
                world_comm.recv(status.source(), CHILD_SELEC, m);
                evolve_into_next_depth(m);
            } else {
                world_comm.recv(status.source(), status.tag());
                switch (status.tag()) {
                case BROADCAST_TREE:
                    slave_broadcast_tree();
                    break;
                case REDUCE_SIMS:
                    slave_reduce_sims();
                    break;
                case TS_START:
                    stop = false;
                    for (int i = 0; i < workers; ++i) {
                        local_oq_vec[i].reset();
                        local_iq_vec[i].reset();
                    }
#ifdef UCT_TREESPLIT_CLEAN_BEFORE_CALC
                    treesplit_node::remove_transmap_useless_entries();
#endif
                    for (int i = 0; i < workers; ++i) {
                        thread_vec[i] = thread(&uct_treesplit_player::worker_thread, this, i);
                    }
                    break;
                case TS_STOP:
                    stop = true;
                    for (auto &&t : thread_vec) {
                        t.join();
                    }
                    for (auto &&req : pending_requests) {
                        if (!req.test()) {
                            req.cancel();
                        }
                    }
                    pending_requests.clear();
                    break;
                case TS_BEST_CHILD:
#ifdef _DEBUG
                    cout << "[" << rank << "] gathering best children" << endl;
#endif
                    mpi::gather(world_comm, dynamic_pointer_cast<treesplit_node>(root)->get_best_child_msg(), 0);
                    break;
                case EXIT:
                    return;
                default:
                    cerr << "unknown tag: " << status.tag() << endl;
                    throw invalid_argument("received an invalid mpi tag in do_slave_job()");
                }
            }
        } else if (stop.load(std::memory_order_relaxed)) {//only sleep if TS is stopped
            static const milliseconds nap(50);
            this_thread::sleep_for(nap);
        } else {//do IO job
            auto q = max_element(local_oq_vec.begin(), local_oq_vec.end(), [](const lf_queue &x, const lf_queue &y){
                return x.size() < y.size();
            });
            if (!q->empty()) {
                auto omsg = q->front();
                q->pop();
                if (get<0>(omsg) == -1) {
#ifdef _DEBUG
                    cout << "[" << rank << "] send out a duplicate message" << endl;
#endif
                    for (int i = 0; i < world_size; ++i) {
                        if (i == rank) {
                            continue;
                        }
                        pending_requests.push_back(world_comm.isend(i, TS_MSG, omsg));
                    }
                } else {
#ifdef _DEBUG
                    cout << "[" << rank << "] send out an update message" << endl;
#endif
                    pending_requests.push_back(world_comm.isend(get<0>(omsg), TS_MSG, omsg));
                }
            }
        }
    } while (true);
}

void uct_treesplit_player::worker_thread(const int &id)
{
    queue<treesplit_node::msg_type>().swap(treesplit_node::output_queue);
    do {
        if (local_iq_vec[id].empty()) {
            root->select();
            selects++;
            while (!treesplit_node::output_queue.empty()) {
                local_oq_vec[id].push(treesplit_node::output_queue.front());
                treesplit_node::output_queue.pop();
            }
        } else {
            treesplit_node::handle_message(local_iq_vec[id].front());
            local_iq_vec[id].pop();
        }
    } while (!stop.load(std::memory_order_relaxed));
}
