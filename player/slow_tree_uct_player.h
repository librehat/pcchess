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
    explicit slow_tree_uct_player(long int sync_period_ms = 1000, bool red = true);
    ~slow_tree_uct_player();

    bool think_next_move(pos_move &_move, const board &bd, unsigned int no_eat_half_rounds, const std::vector<pos_move> &banmoves);
    void opponent_moved(const pos_move &m);
    std::int64_t get_total_simulations() const;

    void do_slave_job();

private:
    long int sync_period;

    void master_send_order(const int &tag) const;

    void slave_opponent_moved();
    void slave_select_child();
    void slave_compute();
    void slave_broadcast_tree();

    void sync_tree();

    static const int TAG_SYNC;
    static const int TAG_OPPMOV;
    static const int TAG_OPPMOV_DATA;
    static const int TAG_CHILD_SELEC;
    static const int TAG_CHILD_SELEC_DATA;
    static const int TAG_COMP_LOOP;
    static const int TAG_COMP_FINISH;
    static const int TAG_BROADCAST_TREE;
    static const int TAG_REDUCE_SIMS;
    static const int TAG_EXIT;

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
