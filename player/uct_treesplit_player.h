#ifndef UCT_TREESPLIT_PLAYER_H
#define UCT_TREESPLIT_PLAYER_H

#include "root_uct_player.h"
#include "treesplit_node.h"
#include "utils/lock_free_queue.hpp"
#include <thread>

class uct_treesplit_player : public root_uct_player
{
public:
    /*
     * this algorithm allocate one thread on one CPU core (or a hyperthread unit)
     * by default, this constructor will try to use thread::hardware_concurrency() as the CPU cores.
     * if that information is unavailable, then it'll assume this machine has a quad-core CPU
     */
    explicit uct_treesplit_player(int cpu_cores = 0, bool red = true);

    bool think_next_move(pos_move &_move, const board &bd, std::uint8_t no_eat_half_rounds, const std::vector<pos_move> &banmoves);
    void opponent_moved(const pos_move &m);
    void do_slave_job();
    inline const int &worker_size() const { return workers; }
    int thread_size() { return thread_vec.size() + 1; }

private:
    typedef lock_free_queue<treesplit_node::msg_type, 262144> lf_queue;

    boost::mpi::communicator slave_comm;

    int workers;
    std::vector<boost::mpi::request> pending_requests;
    std::vector<std::thread> thread_vec;
    std::vector<lf_queue> local_oq_vec;
    std::vector<lf_queue> local_iq_vec;

    void evolve_into_next_depth(const pos_move &m);//can only be called by master
    void slave_stop();
    void slave_select_child();

    void main_thread_start();//basically just reset the stop flag, create worker threads and start io work
    void worker_thread(const int &id);

    std::atomic_flag compute_flag;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(root_uct_player);
        ar & BOOST_SERIALIZATION_NVP(workers);
    }
};

#endif //UCT_TREESPLIT_PLAYER_H
