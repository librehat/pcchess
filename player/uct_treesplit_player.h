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

    std::int64_t get_total_simulations() const;

private:
    int workers;
    std::vector<std::thread> thread_vec;
    std::vector<lock_free_queue<treesplit_node::msg_type> > local_oq_vec;

    void evolve_into_next_depth(const pos_move &m);//can only be called by master
    void slave_init();
    void slave_select_child();

    void main_thread_start();//basically just reset the stop flag, create worker threads and start io work
    void worker_thread(const int &id);

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
