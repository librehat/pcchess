/*
 * This player is based on the Monte Carlo Tree Search algorithm
 * with UCT opimisation in search step
 */

#ifndef UCT_PLAYER_H
#define UCT_PLAYER_H

#include "abstract_player.h"
#include "node.h"
#include <chrono>

class uct_player : public abstract_player
{
public:
    uct_player(double _think_time, const abstract_player* const _opp, bool opposite);//think_time: seconds
    virtual ~uct_player();

    virtual bool think_next_move(pos_move &_move, const board &);
    virtual void opponent_moved(const pos_move &m);
    virtual int get_total_simulations() const;

protected:
    std::chrono::duration<double> think_time;//the maximum period for each move
    node *root;
};

#endif // UCT_PLAYER_H
