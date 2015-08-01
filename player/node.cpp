#include "node.h"
#include "../core/board.h"
#include "../core/game.h"
#include "random_player.h"
#include <stdexcept>
#include <cmath>
#include <iostream>

using namespace std;

atomic<int64_t> node::total_simulations(0);
atomic_int node::root_depth(0);
atomic_int node::max_depth(50);//rounds = depth / 2 //TODO tuned
const int node::select_threshold = 100;
const double node::uct_constant = 0.7;//TODO tuned

node::node(const string &fen, const pos_move &_mov, bool _my_turn, bool is_red_side, uint8_t noeat_half_rounds, node *_parent) :
	my_turn(_my_turn),
    red_side(is_red_side),
    parent(_parent),
    current_fen(fen),
    my_move(_mov),
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

node::node(const string &fen, bool is_red_side, uint8_t noeat_half_rounds) :
    my_turn(true),
    red_side(is_red_side),
    parent(nullptr),
    current_fen(fen),
    depth(0),
    visits(0),
    scores(0),
    no_eat_half_rounds(noeat_half_rounds)
{}

node::~node()
{}

node::node_ptr node::make_shallow_copy() const
{
    node_ptr n(new node(current_fen, my_move, my_turn, red_side, no_eat_half_rounds, parent));
    n->depth = depth.load();
    return n;
}

node::node_ptr node::make_shallow_copy_with_children() const
{
    node_ptr n = this->make_shallow_copy();
    for (auto it = children.begin(); it != children.end(); ++it) {
        node_ptr c = (*it)->make_shallow_copy();
        c->parent = n.get();
        n->children.push_back(c);
    }
    return n;
}

node::node_ptr node::gen_child_with_a_move(const pos_move &m)
{
    random_player tr(true), tb(false);
    game updater_sim(&tr, &tb, no_eat_half_rounds);
    updater_sim.parse_fen(current_fen);
    updater_sim.move_piece(m);
    node_ptr child(new node(updater_sim.get_fen(), m, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat(), this));
    return child;
}

double node::get_value() const
{
    return static_cast<double>(scores) / static_cast<double>(visits);
}

double node::get_uct_val() const
{
	if (parent) {
        return get_value() + uct_constant * sqrt(log(parent->visits.load()) / visits);
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
            return (*get_best_child_uct())->select();
        } else {
            return (*get_worst_child_uct())->select();
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

    node_ptr child;
    auto child_iter = find_child(next_move);
    if (child_iter == children.end()) {
        random_player tr(true), tb(false);
        game updater_sim(&tr, &tb, no_eat_half_rounds);
        updater_sim.parse_fen(current_fen);
        updater_sim.move_piece(next_move);
        if (my_turn) {
            if (updater_sim.is_player_in_check(red_side)) {
                return;//we're checked! don't make this move
            }
        }
        child = node_ptr(new node(updater_sim.get_fen(), next_move, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat(), this));
        children.push_back(child);
    } else {
        child = *child_iter;
    }
    child->expand(hist, score);
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
        auto src_it = find_child((*target_it)->my_move);
        if (src_it != children.end()) {
            (*src_it)->merge(**target_it, average_mode);
            b.children.erase(target_it);
        } else {
            node::node_ptr n = b.release_child(target_it);
            n->parent = this;
            children.push_back(n);
        }
    }
}

int node::children_size() const
{
    return children.size();
}

void node::backpropagate(const int &score, const int &vis)
{
    if (parent) {
        parent->visits += vis;
        parent->scores += score;
        parent->backpropagate(score, vis);
    }
}

node::node_ptr node::release_child(node::iterator i)
{
    assert(i != children.end());
    node_ptr c = *i;
    c->parent = nullptr;
    children.erase(i);
    return c;
}

node::iterator node::get_best_child()
{
    return max_element(children.begin(), children.end(), compare_visits);
}

node::iterator node::get_best_child_uct()
{
    return max_element(children.begin(), children.end(), compare_uct);
}

node::iterator node::get_worst_child_uct()
{
    return min_element(children.begin(), children.end(), compare_uct);
}

node::iterator node::find_child(const pos_move &m)
{
    return find_if(children.begin(), children.end(), [&m](const node_ptr &n){
        return n->my_move == m;
    });
}

bool node::is_same_place_in_tree(const node &b) const
{
    /* true if they should be in the same place */
    return !(my_turn != b.my_turn || depth.load() != b.depth.load() || current_fen != b.current_fen || red_side != b.red_side || no_eat_half_rounds.load() != b.no_eat_half_rounds.load() || my_move.load() != b.my_move.load());
}

bool node::is_basically_the_same(const node &b) const
{
    return !(!is_same_place_in_tree(b) || visits.load() != b.visits.load() || scores.load() != b.scores.load() || children.size() != b.children.size());
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

void node::set_root_depth(const node_ptr r)
{
    if(!r) {
        throw invalid_argument("pointer r is nullptr");
    }
    root_depth = r->depth.load();
}

void node::set_max_depth(const int &d)
{
    max_depth = d;
}

bool node::compare_visits(const std::vector<node_ptr>::value_type &x, const std::vector<node_ptr>::value_type &y)
{
    return x->visits < y->visits;
}

bool node::compare_uct(const std::vector<node_ptr>::value_type &x, const std::vector<node_ptr>::value_type &y)
{
    return x->get_uct_val() < y->get_uct_val();
}
