#include "node.h"
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

    our = new random_player(*(parent->our), t_board);
    opp = new random_player(*(parent->opp), t_board);
}

node::node(const abstract_player *_our, const abstract_player *_opp) :
    parent(nullptr),
    visits(0),
    scores(0)
{
    if (!_our || !_opp) {
        throw runtime_error("Error. Null player pointer is used for node constructor.");
    }

    our = new random_player(*_our, t_board);
    opp = new random_player(*_opp, t_board);
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

void node::play_random_game()
{
    game sim_game(our, opp, t_board);
    abstract_player* winner = sim_game.playout();
    int res = 0;
    if (winner == our) {
        res = 1;
    } else if (winner == opp) {
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
