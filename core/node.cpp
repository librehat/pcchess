#include "node.h"
#include "board.h"
#include "game.h"
#include "random_player.h"
#include <stdexcept>
#include <cmath>
#include <iostream>

using namespace std;

int node::total_simulations = 0;
const int node::select_threshold = 100;
const double node::uct_constant = 0.7;//need to be tuned based on experiments

node::node(abstract_player *_our, abstract_player *_opp, bool _my_turn, node *_parent) :
	my_turn(_my_turn),
    parent(_parent),
    our_curr(_our),
    opp_curr(_opp),
    visits(0),
    scores(0)
{
    if (!_our || !_opp) {
        throw invalid_argument("Error. Null player pointer is used for node constructor.");
    }
}

node::~node()
{
    for(auto &&child : children) {
        delete child;
    }
    delete our_curr;
    delete opp_curr;
}

int node::get_visits() const
{
    return visits;
}

int node::get_scores() const
{
    return scores;
}

double node::get_value() const
{
    return static_cast<double>(scores) / static_cast<double>(visits);
}

const pos_move& node::get_our_move() const
{
    return our_move;
}

const pos_move& node::get_opp_move() const
{
    return opp_move;
}

double node::get_uct_val() const
{
	if (parent) {
		return get_value() + uct_constant * sqrt(log(parent->get_visits()) / visits);
	} else {
		return 0;
	}
}

void node::set_our_move(const pos_move &m)
{
    our_move = m;
}

void node::set_opp_move(const pos_move &m)
{
    opp_move = m;
}

bool node::select()
{
#ifdef _DEBUG
	cout << "SELECTION step" << endl;
#endif

    if (visits > select_threshold && !children.empty()) {
        return get_best_child_uct()->select();
    } else {
        return simulate();
    }
}

void node::expand(list<pos_move> &our_hist, list<pos_move> &opp_hist, const int &score)
{
#ifdef _DEBUG
	cout << "EXPANSION step" << endl;
#endif

	visits++;
	scores += score;

	pos_move next_move;
	if (my_turn) {
	    if (our_hist.empty()) {
#ifdef _DEBUG
            cout << "our_hist is empty" << endl;
#endif
	        return;
	    }
	    next_move = our_hist.back();
	    our_hist.pop_back();
	} else {
	    if (opp_hist.empty()) {
#ifdef _DEBUG
            cout << "opp_hist is empty" << endl;
#endif
	        return;
	    }
	    next_move = opp_hist.back();
	    opp_hist.pop_back();
	}

	node* child = find_child(next_move);
	if (!child) {
        abstract_player* n_our = new random_player(*our_curr);
        abstract_player* n_opp = new random_player(*opp_curr);

        game updater_sim(n_our, n_opp);
        updater_sim.move_piece(next_move);

        child = new node(n_our, n_opp, !my_turn, this);
	    child->set_our_move(my_turn ? next_move : our_move);
	    child->set_opp_move(my_turn ? opp_move : next_move);
        children.push_back(child);
#ifdef _DEBUG
        cout << "A new child node is created" << endl;
#endif
	}
	child->expand(our_hist, opp_hist, score);
}

bool node::simulate()
{
#ifdef _DEBUG
	cout << "SIMULATION step" << endl;
#endif
    total_simulations++;
    random_player t_our(*our_curr);
    random_player t_opp(*opp_curr);

	//need to clear the history so the history would contains only the simulation part
    t_our.clear_history();
    t_opp.clear_history();

    game sim_game(&t_our, &t_opp);
    abstract_player* winner = sim_game.playout(my_turn);
    int result = 0;
    if (winner == &t_our) {
        result = 1;
    } else if (winner == &t_opp){
        result = -1;
    }

#ifdef _DEBUG
    cout << "SIMULATION result: " << result << endl;
#endif

	//we need to make copies here because expand will modify the argument variables
    list<pos_move> our_hist = t_our.get_history();
    list<pos_move> opp_hist = t_opp.get_history();

    if ((our_hist.empty() && my_turn) || (opp_hist.empty() && !my_turn)) {//can't expand the tree if the current player can't move
        visits++;
        scores += result;
        return false;
    } else {
        expand(our_hist, opp_hist, result);//this very node is definitely the parental node
        return true;
    }
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

node* node::get_best_child_uct() const
{
    if (children.empty()) {
        return nullptr;
    }

    node* best_child = children.front();
    auto best_uct = children.front()->get_uct_val();
    auto it = children.cbegin();
    it++;
    for (; it != children.end(); ++it) {
        auto uct = (*it)->get_uct_val();
        if (uct > best_uct) {
            best_child = *it;
            best_uct = uct;
        }
    }
    return best_child;
}

void node::remove_child(node *c)
{
    children.remove(c);
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
    parent->remove_child(this);
    parent = nullptr;
}

node* node::find_child(const pos_move &m)
{
    for (auto &&child : children) {
        if (my_turn) {
            if (child->get_our_move() == m) {
                return child;
            }
        } else {
            if (child->get_opp_move() == m) {
                return child;
            }
        }
    }
    return nullptr;
}

int node::get_total_simulations()
{
    return total_simulations;
}
