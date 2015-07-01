#include "mcts_player.h"

using namespace std;

mcts_player::mcts_player(int depth, const abstract_player *opp, board &bd) :
    abstract_player(bd),
    search_depth(depth)
{
    root = new node(this, opp, true);
}

mcts_player::~mcts_player()
{
    delete root;
}

bool mcts_player::think_next_move(pos_move &_move)
{
    //TODO
    node* best_child = root->get_best_child();
    if (best_child) {
        _move = best_child->get_our_move();
        return true;
    }
    return false;
}

