#ifndef SLOW_TREE_UCT_PLAYER_H
#define SLOW_TREE_UCT_PLAYER_H

#include "../core/uct_player.h"
#include <boost/mpi.hpp>

class slow_tree_uct_player : public uct_player
{
public:
    slow_tree_uct_player(double _think_time = 1, const abstract_player* const _opp = nullptr, bool opposite = false);

    bool think_next_move(pos_move &_move, const board &);
    void opponent_moved(const pos_move &m);
    int get_total_simulations() const;

private:
    void init_mpi_nodes();//distribute the tree (the root node) among all nodes

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<uct_player>(*this);
    }

    static boost::mpi::communicator world_comm;
};

#endif //SLOW_TREE_UCT_PLAYER_H
