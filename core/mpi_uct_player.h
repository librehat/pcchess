#ifndef MPI_UCT_PLAYER_H
#define MPI_UCT_PLAYER_H

#include "uct_player.h"

class mpi_uct_player : public uct_player
{
public:
    mpi_uct_player(double _think_time, const abstract_player* const _opp, bool opposite, int _compute_nodes);

    bool think_next_move(pos_move &_move, const board &);
    void opponent_moved(const pos_move &m);
    int get_total_simulations() const;

private:
    int compute_nodes;
};

#endif //MPI_UCT_PLAYER_H
