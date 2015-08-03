#ifndef UCT_TREESPLIT_PLAYER_H
#define UCT_TREESPLIT_PLAYER_H

#include "root_uct_player.h"
#include "treesplit_node.h"
#include <thread>

class uct_treesplit_player : public root_uct_player
{
public:
    explicit uct_treesplit_player(int cpu_cores = 2, bool red = true);//this algorithm allocate one thread on one CPU core

    bool think_next_move(pos_move &_move, const board &bd, std::uint8_t no_eat_half_rounds, const std::vector<pos_move> &banmoves);
    void opponent_moved(const pos_move &m);
    void do_slave_job();

    std::int64_t get_total_simulations() const;

private:
    int workers;
    std::vector<std::thread> thread_vec;
    std::vector<thread_safe_queue<treesplit_node::msg_type> > iq_vec;//one worker thread one queue
    thread_safe_queue<treesplit_node::msg_type> oq;

    void evolve_into_next_depth(const pos_move &m);//can only be called by master
    void slave_select_child();

    void io_work();//this function should be called only by the main thread (other threads are workers)
    void worker_thread(const int &);

    static std::atomic_bool stop;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(root_uct_player);
        ar & BOOST_SERIALIZATION_NVP(workers);
    }
};

#endif //UCT_TREESPLIT_PLAYER_H
