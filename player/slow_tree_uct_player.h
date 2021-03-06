/*
 * slow_tree_uct_player implements slow-tree-parallelisation
 * each MPI node mantains an independent tree with occasional synchronisations
 */
#ifndef SLOW_TREE_UCT_PLAYER_H
#define SLOW_TREE_UCT_PLAYER_H

#include "root_uct_player.h"

class slow_tree_uct_player : public root_uct_player
{
public:
    explicit slow_tree_uct_player(long int sync_period_ms = 500, bool red = true);

    bool think_next_move(pos_move &_move, const board &bd, std::uint8_t no_eat_half_rounds, const std::vector<pos_move> &banmoves);
    void opponent_moved(const pos_move &m);

    void do_slave_job();

private:
    long int sync_period;

    void sync_tree();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(root_uct_player);
        ar & BOOST_SERIALIZATION_NVP(sync_period);
    }
};

#endif //SLOW_TREE_UCT_PLAYER_H
