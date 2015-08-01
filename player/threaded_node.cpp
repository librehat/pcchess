#include "threaded_node.h"
#include "random_player.h"
#include "../core/game.h"
#include <iostream>

using namespace std;

threaded_node::threaded_node(const string &fen, const pos_move &mov, bool _my_turn, bool is_red_side, uint8_t noeat_half_rounds, node *_parent) :
    node(fen, mov, _my_turn, is_red_side, noeat_half_rounds, _parent)
{}

threaded_node::threaded_node(const string &fen, bool is_red_side, uint8_t noeat_half_rounds) :
    node(fen, is_red_side, noeat_half_rounds)
{}

bool threaded_node::select()
{
    if (visits > select_threshold && !children.empty()) {
        node_ptr bc;
        children_mutex.lock();
        if (my_turn) {
            bc = *get_best_child_uct();
        } else {
            bc = *get_worst_child_uct();
        }
        children_mutex.unlock();
        return bc->select();
    } else {
        return simulate();
    }
}

void threaded_node::expand(deque<pos_move> &hist, const int &score)
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
    children_mutex.lock();
    auto child_iter = find_child(next_move);
    if (child_iter != children.end()) {
        child = *child_iter;
    } else {
        children_mutex.unlock();
        random_player tr(true), tb(false);
        game updater_sim(&tr, &tb, no_eat_half_rounds);
        updater_sim.parse_fen(current_fen);
        updater_sim.move_piece(next_move);
        if (my_turn) {
            if (updater_sim.is_player_in_check(red_side)) {
                return;//we're checked! don't make this move
            }
        }
        child = node_ptr(new threaded_node(updater_sim.get_fen(), next_move, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat(), this));
        children_mutex.lock();
        children.push_back(child);
    }
    children_mutex.unlock();
    child->expand(hist, score);
}

bool threaded_node::simulate()
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
