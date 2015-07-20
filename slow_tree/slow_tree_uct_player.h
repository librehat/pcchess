#ifndef SLOW_TREE_UCT_PLAYER_H
#define SLOW_TREE_UCT_PLAYER_H

#include "../core/uct_player.h"
#include <boost/mpi.hpp>

class slow_tree_uct_player : public uct_player
{
public:
    slow_tree_uct_player(const abstract_player* const _opp = nullptr, bool opposite = false);

    bool think_next_move(pos_move &_move, const board &);
    void opponent_moved(const pos_move &m);

    void do_slave_job();

    static const int TAG_SYNC = 1;
    static const int TAG_OPPMOV = 10;
    static const int TAG_OPPMOV_DATA = 11;
    static const int TAG_CHILD_SELEC = 20;
    static const int TAG_CHILD_SELEC_DATA = 21;
    static const int TAG_COMP = 30;
    static const int TAG_ERASE = 90;
    static const int TAG_EXIT = 0;

private:
    void slave_opponent_moved();
    void slave_select_child();
    void slave_compute();

    void broadcast_tree();
    void sync_tree();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(uct_player);
    }

    static boost::mpi::communicator world_comm;
};

#endif //SLOW_TREE_UCT_PLAYER_H
