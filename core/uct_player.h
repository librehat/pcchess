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
    uct_player(double _think_time, bool first_hand, const abstract_player* const _opp);//think_time: seconds
    virtual ~uct_player();

    virtual bool think_next_move(pos_move &_move, const board &m_board);
    virtual void opponent_moved(const pos_move &m);

protected:
    std::chrono::duration<double> think_time;//the maximum period for each move
    const abstract_player* const opp;
    const bool firsthand;
    node *root;
};

#endif // UCT_PLAYER_H
