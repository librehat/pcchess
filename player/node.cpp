#include "node.h"
#include "../core/board.h"
#include "../core/game.h"
#include "random_player.h"
#include <stdexcept>
#include <cmath>
#include <iostream>

using namespace std;

int64_t node::total_simulations = 0;
int node::root_depth = 0;
int node::max_depth = 50;//rounds = depth / 2 //TODO tuned
const int node::select_threshold = 100;
const double node::uct_constant = 0.7;//TODO tuned

node::node(const string &fen, bool _my_turn, bool is_red_side, int8_t noeat_half_rounds, node *_parent) :
	my_turn(_my_turn),
    red_side(is_red_side),
    parent(_parent),
    current_fen(fen),
    visits(0),
    scores(0),
    no_eat_half_rounds(noeat_half_rounds)
{
    if (parent) {
        depth = parent->depth + 1;
    } else {
        depth = 0;
    }
}

node::~node()
{
    for (auto &&c : children) {
        delete c.second;
    }
}

node* node::make_shallow_copy() const
{
    node* n = new node(current_fen, my_turn, red_side, no_eat_half_rounds, parent);
    n->depth = depth;
    return n;
}

node* node::make_shallow_copy_with_children() const
{
    node* n = this->make_shallow_copy();
    for (auto it = children.begin(); it != children.end(); ++it) {
        node* c = it->second->make_shallow_copy();
        c->parent = n;
        n->children.emplace(it->first, c);
    }
    return n;
}

node* node::gen_child_with_a_move(const pos_move &m)
{
    random_player tr(true), tb(false);
    game updater_sim(&tr, &tb, no_eat_half_rounds);
    updater_sim.parse_fen(current_fen);
    updater_sim.move_piece(m);
    node *child = new node(updater_sim.get_fen(), !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat(), this);
    return child;
}

double node::get_value() const
{
    return static_cast<double>(scores) / static_cast<double>(visits);
}

double node::get_uct_val() const
{
	if (parent) {
        return get_value() + uct_constant * sqrt(log(parent->visits) / visits);
	} else {
		return 0;
	}
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
            return get_best_child_uct()->second->select();
        } else {
            return get_worst_child_uct()->second->select();
        }
    } else {
        return simulate();
    }
}

void node::expand(deque<pos_move> &hist, const int &score)
{
	visits++;
	scores += score;
    if (depth - root_depth > max_depth) {
        return;
    }
    if (hist.empty()) {
        return;
    }
    pos_move next_move = hist.back();
    hist.pop_back();

    auto child_iter = children.find(next_move);
    if (child_iter == children.end()) {
        node *child = gen_child_with_a_move(next_move);
        children.emplace(next_move, child);
        child->expand(hist, score);
    } else {
        child_iter->second->expand(hist, score);
    }
}

bool node::simulate()
{
    total_simulations++;

    random_player tr(true), tb(false);
    game sim_game(&tr, &tb, no_eat_half_rounds);
    sim_game.parse_fen(current_fen);
    abstract_player* winner = sim_game.playout(!(my_turn ^ red_side));

    int result = 0;
    if (winner == &tr) {
        result = red_side ? 1 : -1;
    } else if (winner == &tb) {
        result = red_side ? -1 : 1;
    }

    auto hist = sim_game.get_history();
    if (hist.empty()) {//can't expand the tree if the current player can't move
        visits++;
        scores += result;
        backpropagate(result);
        return false;
    } else {
        expand(hist, result);//this very node is definitely the parental node
        backpropagate(result);
        return true;
    }
}

void node::merge(node &b, bool average_mode)
{
    assert(is_same_place_in_tree(b));
    if (average_mode) {
        visits += ((b.visits - visits) / 2);
        scores += ((b.scores - scores) / 2);
    } else {
        visits += b.visits;
        scores += b.scores;
    }

    /* The node b will give its children to us, of which is either merged or pushed back as a new child */
    for (auto target_it = b.children.begin(); target_it != b.children.end(); target_it = b.children.begin()) {
        auto src_it = children.find(target_it->first);
        if (src_it != children.end()) {
            src_it->second->merge(*(target_it->second), average_mode);
            delete target_it->second;
        } else {
            target_it->second->parent = this;
            children.emplace(target_it->first, target_it->second);
        }
        b.children.erase(target_it);
    }
}

int node::children_size() const
{
    return children.size();
}

void node::backpropagate(const int &score)
{
    if (parent) {
        parent->visits += 1;
        parent->scores += score;
        parent->backpropagate(score);
    }
}

node* node::release_child(node::node_iterator i)
{
    assert(i != children.end());
    node* c = i->second;
    c->parent = nullptr;
    children.erase(i);
    return c;
}

node::node_iterator node::get_best_child()
{
    return max_element(children.begin(), children.end(), compare_visits);
}

node::node_iterator node::get_best_child_uct()
{
    return max_element(children.begin(), children.end(), compare_uct);
}

node::node_iterator node::get_worst_child_uct()
{
    return min_element(children.begin(), children.end(), compare_uct);
}

bool node::is_same_place_in_tree(const node &b) const
{
    /* true if they should be in the same place */
    return !(my_turn != b.my_turn || depth != b.depth || current_fen != b.current_fen || red_side != b.red_side || no_eat_half_rounds != b.no_eat_half_rounds);
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
    return parent != b.parent || children != b.children || !is_basically_the_same(b);
}

int64_t node::get_total_simulations()
{
    return total_simulations;
}

void node::set_root_depth(const node * const r)
{
    if(!r) {
        throw invalid_argument("pointer r is nullptr");
    }
    root_depth = r->depth;
}

void node::set_max_depth(const int &d)
{
    max_depth = d;
}

bool node::compare_visits(const unordered_map<pos_move, node *>::value_type &x, const unordered_map<pos_move, node *>::value_type &y)
{
    return x.second->visits < y.second->visits;
}

bool node::compare_uct(const unordered_map<pos_move, node *>::value_type &x, const unordered_map<pos_move, node *>::value_type &y)
{
    return x.second->get_uct_val() < y.second->get_uct_val();
}
