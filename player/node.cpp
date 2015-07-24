#include "node.h"
#include "../core/board.h"
#include "../core/game.h"
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
    if (parent) {
        depth = parent->depth + 1;
    } else {
        depth = 0;
    }
}

node::~node()
{
    children.clear();
    if (our_curr) {
        delete our_curr;
    }
    if (opp_curr) {
        delete opp_curr;
    }
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
        return get_value() + uct_constant * sqrt(log(parent->visits) / visits);
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
    if (visits > select_threshold && !children.empty()) {
        /*
         * if the children represent opponent's moves, because
         * the opponent would possibly make a move giving them
         * a better chance, which is basically our worst case,
         * the child returned should be the worst instead of
         * best in this situation.
         */
        if (my_turn) {
            return get_best_child_uct()->select();
        } else {
            return get_worst_child_uct()->select();
        }
    } else {
        return simulate();
    }
}

void node::expand(list<pos_move> &our_hist, list<pos_move> &opp_hist, const int &score)
{
	visits++;
	scores += score;

	pos_move next_move;
	if (my_turn) {
	    if (our_hist.empty()) {
	        return;
	    }
	    next_move = our_hist.back();
	    our_hist.pop_back();
	} else {
	    if (opp_hist.empty()) {
	        return;
	    }
	    next_move = opp_hist.back();
	    opp_hist.pop_back();
	}

    auto child_iter = find_child(next_move);
    if (child_iter == children.end()) {
        abstract_player* n_our = new random_player(*our_curr);
        abstract_player* n_opp = new random_player(*opp_curr);
        bool is_red = !n_our->is_opposite();

        game updater_sim(is_red ? n_our : n_opp, is_red ? n_opp : n_our);
        updater_sim.move_piece(next_move);

        node *child = new node(n_our, n_opp, !my_turn, this);
	    child->set_our_move(my_turn ? next_move : our_move);
	    child->set_opp_move(my_turn ? opp_move : next_move);
        children.push_back(child);
    } else {
        child_iter->expand(our_hist, opp_hist, score);
    }
}

bool node::simulate()
{
    total_simulations++;
    random_player t_our(*our_curr);
    random_player t_opp(*opp_curr);

	//need to clear the history so the history would contains only the simulation part
    t_our.clear_history();
    t_opp.clear_history();
    bool is_red = !t_our.is_opposite();

    game sim_game(is_red ? &t_our : &t_opp, is_red ? &t_opp : &t_our);
    abstract_player* winner = sim_game.playout(my_turn && is_red);
    int result = 0;
    if (winner == &t_our) {
        result = 1;
    } else if (winner == &t_opp){
        result = -1;
    }

	//we need to make copies here because expand will modify the argument variables
    list<pos_move> our_hist = t_our.get_history();
    list<pos_move> opp_hist = t_opp.get_history();

    if ((our_hist.empty() && my_turn) || (opp_hist.empty() && !my_turn)) {//can't expand the tree if the current player can't move
        visits++;
        scores += result;
        backpropagate(result);
        return false;
    } else {
        expand(our_hist, opp_hist, result);//this very node is definitely the parental node
        backpropagate(result);
        return true;
    }
}

void node::merge(node &b)//FIXME: somehow it causes MPI address not mapped error
{
    assert(is_same_place_in_tree(b));
    visits += b.visits;
    scores += b.scores;

    /* The node b will give its children to us, of which is either merged or pushed back as a new child */
    for (auto target_it = b.children.begin(); target_it != b.children.end(); target_it = b.children.begin()) {
        bool merged = false;
        for (auto src_it = children.begin(); src_it != children.end(); ++src_it) {
            if (src_it->is_same_place_in_tree(*target_it)) {//we already have an equivalent node
                merged = true;
                node* target = b.release_child(target_it);
                src_it->merge(*target);
                delete target;
                break;
            }
        }

        //this node is **new** to us
        if (!merged) {
            node* target = b.release_child(target_it);
            target->parent = this;
            children.push_back(target);
        }
    }
}

node_iterator node::child_end()
{
    return children.end();
}

int node::children_size() const
{
    return children.size();
}

void node::backpropagate(const int &score)
{
    if (parent) {
        parent->scores += score;//TODO should we backpropagate visits?
        parent->backpropagate(score);
    }
}

node* node::release_child(node_iterator i)
{
    assert(i != children.end());
    node* c = children.release(i).release();
    c->parent = nullptr;
    return c;
}

node_iterator node::get_best_child()
{
    auto best = children.end();
    int current_max = 0;
    for (auto it = children.begin(); it != children.end(); ++it) {
        if (it->visits > current_max) {
            best = it;
            current_max = it->visits;
        }
    }
    return best;
}

node_iterator node::get_best_child_uct()
{
    if (children.empty()) {
        return children.end();
    }

    auto best_child = children.begin();
    auto best_uct = children.front().get_uct_val();
    auto it = children.begin();
    it++;
    for (; it != children.end(); ++it) {
        auto uct = it->get_uct_val();
        if (uct > best_uct) {
            best_child = it;
            best_uct = uct;
        }
    }
    return best_child;
}

node_iterator node::get_worst_child_uct()
{
    if (children.empty()) {
        return children.end();
    }

    auto worst_child = children.begin();
    auto worst_uct = children.front().get_uct_val();
    auto it = children.begin();
    it++;
    for (; it != children.end(); ++it) {
        auto uct = it->get_uct_val();
        if (uct < worst_uct) {
            worst_child = it;
            worst_uct = uct;
        }
    }
    return worst_child;
}

node_iterator node::find_child(const pos_move &m)
{
    for (auto it = children.begin(); it != children.end(); ++it) {
        if (my_turn) {
            if (it->our_move == m) {
                return it;
            }
        } else {
            if (it->opp_move == m) {
                return it;
            }
        }
    }
    return children.end();
}

bool node::is_same_place_in_tree(const node &b) const
{
    /* true if they should be in the same place */
    return !(my_turn != b.my_turn || depth != b.depth || our_move != b.our_move || opp_move != b.opp_move);
}

bool node::is_basically_the_same(const node &b) const
{
    return !(!is_same_place_in_tree(b) || visits != b.visits || scores != b.scores || children.size() != b.children.size());
}

bool node::operator ==(const node &b) const
{
    return !(*this != b);
}

bool node::operator !=(const node &b) const
{
    return parent != b.parent || children != b.children || our_curr != b.our_curr || opp_curr != b.opp_curr || !is_basically_the_same(b);
}

int node::get_total_simulations()
{
    return total_simulations;
}
