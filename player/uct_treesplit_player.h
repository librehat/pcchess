#ifndef UCT_TREESPLIT_PLAYER_H
#define UCT_TREESPLIT_PLAYER_H

#include "root_uct_player.h"
#include <thread>
#include <atomic>

class uct_treesplit_player : public root_uct_player
{
public:
    explicit uct_treesplit_player(int cores = 4, bool red = true);//this algorithm allocate one thread on one CPU core

    bool think_next_move(pos_move &_move, const board &bd, std::uint8_t no_eat_half_rounds, const std::vector<pos_move> &banmoves);
    void opponent_moved(const pos_move &m);
    void do_slave_job();

    std::int64_t get_total_simulations() const;

private:
    int cpu_cores;
    std::vector<std::thread> thread_vec;

    void evolve_into_next_depth(const pos_move &m);//can only be called by master
    void slave_select_child();

    void io_thread();
    void worker_thread();

    static std::atomic_bool stop;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(root_uct_player);
        ar & BOOST_SERIALIZATION_NVP(cpu_cores);
    }
};

#endif //UCT_TREESPLIT_PLAYER_H
