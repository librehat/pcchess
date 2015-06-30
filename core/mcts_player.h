/*
 * This player is based on the original Monte Carlo Tree Search algorithm
 */

#ifndef MCTS_PLAYER_H
#define MCTS_PLAYER_H

#include "abstract_player.h"
#include "node.h"

class mcts_player : public abstract_player
{
public:
    mcts_player(int depth, const abstract_player *opp, board &bd);
    ~mcts_player();

    bool think_next_move(pos_move &_move);

private:
    int search_depth;
    node *root;
};

#endif // MCTS_PLAYER_H
