#include "mcts_player.h"

using namespace std;

mcts_player::mcts_player(int depth, const abstract_player *opp, board &bd) :
    abstract_player(bd),
    search_depth(depth)
{
    root = new node(this, opp);
}

mcts_player::~mcts_player()
{
    delete root;
}

bool mcts_player::think_next_move(position &from, position &to)
{
    //TODO
}

