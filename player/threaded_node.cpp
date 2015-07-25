#include "threaded_node.h"
#include "random_player.h"
#include "../core/game.h"
#include <iostream>

using namespace std;

threaded_node::threaded_node(abstract_player *_our, abstract_player *_opp, bool _my_turn, unsigned int noeat_half_rounds, const vector<pos_move> &_banmoves, node *_parent) :
    node(_our, _opp, _my_turn, noeat_half_rounds, _banmoves, _parent)
{}

atomic<int> threaded_node::total_simulations(0);

bool threaded_node::select()
{
    if (visits > select_threshold && !children.empty()) {
        children_mutex.lock();
        node_iterator bc;
        if (my_turn) {
            bc = get_best_child_uct();
        } else {
            bc = get_worst_child_uct();
        }
        children_mutex.unlock();
        return bc->select();
    } else {
        return simulate();
    }
}

void threaded_node::expand(deque<pos_move> &hist, const int &score)
{
    value_mutex.lock();
    visits++;
    scores += score;
    value_mutex.unlock();

    if (hist.empty()) {
        return;
    }

    pos_move next_move = hist.back();
    hist.pop_back();

    children_mutex.lock();
    auto child_iter = find_child(next_move);
    if (child_iter != children.end()) {
        children_mutex.unlock();
        child_iter->expand(hist, score);
    } else {
        abstract_player* n_our = new random_player(*our_curr);
        abstract_player* n_opp = new random_player(*opp_curr);
        bool is_red = !n_our->is_opposite();

        game updater_sim(is_red ? n_our : n_opp, is_red ? n_opp : n_our, no_eat_half_rounds);
        updater_sim.move_piece(next_move);

        node *child = new threaded_node(n_our, n_opp, !my_turn, updater_sim.get_half_rounds_since_last_eat(), vector<pos_move>(), this);
        child->set_our_move(my_turn ? next_move : our_move);
        child->set_opp_move(my_turn ? opp_move : next_move);
        children.push_back(child);
        children_mutex.unlock();
        child->expand(hist, score);
    }
}

bool threaded_node::simulate()
{
    total_simulations++;
    random_player t_our(*our_curr);
    random_player t_opp(*opp_curr);
    bool is_red = !t_our.is_opposite();

    game sim_game(is_red ? &t_our : &t_opp, is_red ? &t_opp : &t_our, no_eat_half_rounds, is_red ? banmoves : vector<pos_move>(), is_red ? vector<pos_move>() : banmoves);
    abstract_player* winner = sim_game.playout(my_turn && is_red);
    int result = 0;
    if (winner == &t_our) {
        result = 1;
    } else if (winner == &t_opp){
        result = -1;
    }

    auto hist = sim_game.get_history();
    if (hist.empty()) {//can't expand the tree if the current player can't move
        value_mutex.lock();
        visits++;
        scores += result;
        value_mutex.unlock();
        backpropagate(result);
        return false;
    } else {
        expand(hist, result);//this very node is definitely the parental node
#ifdef _DEBUG
        if (!hist.empty()) {
            cerr << "[threaded_node] hist is not empty after expand step" << endl;
        }
#endif
        backpropagate(result);
        return true;
    }
}

void threaded_node::backpropagate(const int &score)
{
    if (parent) {
        threaded_node* p = dynamic_cast<threaded_node*>(parent);
        p->value_mutex.lock();
        p->visits += 1;
        p->scores += score;
        p->value_mutex.unlock();
        p->backpropagate(score);
    }
}

int threaded_node::get_total_simulations()
{
    return total_simulations;
}
