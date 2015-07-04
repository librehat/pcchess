/*
 * This player is based on the original Monte Carlo Tree Search algorithm
 */

#ifndef MCTS_PLAYER_H
#define MCTS_PLAYER_H

#include "abstract_player.h"
#include "node.h"
#include <chrono>

class mcts_player : public abstract_player
{
public:
    mcts_player(double _think_time, bool first_hand, const abstract_player* const _opp);//think_time: seconds
    ~mcts_player();

    bool think_next_move(pos_move &_move, const board &m_board);
    void opponent_moved(const pos_move &m);

private:
    std::chrono::duration<double> think_time;//the maximum period for each move
    const abstract_player* const opp;
    const bool firsthand;
    node *root;
};

#endif // MCTS_PLAYER_H
