#include "node.h"
#include "board.h"
#include "game.h"
#include "random_player.h"
#include <stdexcept>

using namespace std;

node::node(node *_parent) :
    parent(_parent),
    visits(0),
    scores(0)
{
    if (!_parent) {
        throw runtime_error("Error. Null parent pointer is used for node constructor.");
    }

    our = parent->our;
    opp = parent->opp;
}

node::node(const abstract_player *_our, const abstract_player *_opp) :
    parent(nullptr),
	our(_our),
	opp(_opp),
    visits(0),
    scores(0)
{
    if (!_our || !_opp) {
        throw runtime_error("Error. Null player pointer is used for node constructor.");
    }
}

node::~node()
{
    for(auto &&child : children) {
        delete child;
    }
}

int node::get_visits() const
{
    return visits;
}

int node::get_scores() const
{
    return scores;
}

node* node::get_best_child() const
{
    node* best = nullptr;
    int current_max = 0;
    for (auto &&child : children) {
        if (child->visits > current_max) {
            best = child;
            current_max = child->visits;
        }
    }
    return best;
}

void node::play_random_game(const std::list<pos_move> &moves)
{
	board t_board;
	random_player t_our(*our, t_board);
	random_player t_opp(*opp, t_board);
    game sim_game(&t_our, &t_opp, t_board);

    bool can_continue;
    for(auto it = moves.begin(); it != moves.end(); ++it) {
    	can_continue = sim_game.play_single_move(*it, true);
    	if (!can_continue) {
    		break;
    	}
    }

    abstract_player* winner;
    if (can_continue) {
    	winner = sim_game.playout();
    } else {
    	winner = &t_our;
    }

    int res = 0;
    if (winner == &t_our) {
        res = 1;
    } else if (winner == &t_opp) {
        res = -1;
    }

    backpropagate(res);
}

void node::backpropagate(const int &score)
{
    if (parent) {
        parent->backpropagate(score);
    }
    scores += score;
}

void node::detach()
{
    parent->children.remove(this);
    parent = nullptr;
}
