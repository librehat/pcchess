/*
 * slow_tree_uct_player implements slow-tree-parallelisation
 * each MPI node mantains an independent tree with occasional synchronisations
 */
#ifndef SLOW_TREE_UCT_PLAYER_H
#define SLOW_TREE_UCT_PLAYER_H

#include "uct_player.h"
#include <boost/mpi/communicator.hpp>

class slow_tree_uct_player : public uct_player
{
public:
    explicit slow_tree_uct_player(long int sync_period_ms = 500, bool red = true);
    ~slow_tree_uct_player();

    bool think_next_move(pos_move &_move, const board &bd, unsigned int no_eat_half_rounds, const std::vector<pos_move> &banmoves);
    void opponent_moved(const pos_move &m);
    std::int64_t get_total_simulations() const;

    void do_slave_job();

    static const int TAG_SYNC = 1;
    static const int TAG_OPPMOV = 10;
    static const int TAG_OPPMOV_DATA = 11;
    static const int TAG_CHILD_SELEC = 20;
    static const int TAG_CHILD_SELEC_DATA = 21;
    static const int TAG_COMP_LOOP = 30;
    static const int TAG_COMP_FINISH = 31;
    static const int TAG_BROADCAST_TREE = 40;
    static const int TAG_REDUCE_SIMS = 50;
    static const int TAG_ERASE = 90;
    static const int TAG_EXIT = 0;

private:
    long int sync_period;

    void slave_opponent_moved();
    void slave_select_child();
    void slave_compute();

    void broadcast_tree();
    void sync_tree();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(uct_player);
        ar & BOOST_SERIALIZATION_NVP(sync_period);
    }

    static boost::mpi::communicator world_comm;
};

#endif //SLOW_TREE_UCT_PLAYER_H
