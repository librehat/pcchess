#include "threaded_node.h"
#include "random_player.h"
#include "game.h"
#include <iostream>

using namespace std;

threaded_node::threaded_node(abstract_player *_our, abstract_player *_opp, bool _my_turn, node *_parent) :
    node(_our, _opp, _my_turn, _parent)
{}

bool threaded_node::select()
{
    if (visits > select_threshold && !children.empty()) {
        children_mutex.lock();
        auto bc = get_best_child_uct();
        children_mutex.unlock();
        return bc->select();
    } else {
        return simulate();
    }
}

void threaded_node::expand(list<pos_move> &our_hist, list<pos_move> &opp_hist, const int &score)
{
    value_mutex.lock();
    visits++;
    scores += score;
    value_mutex.unlock();

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

    children_mutex.lock();
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
    }
    children_mutex.unlock();
    child->expand(our_hist, opp_hist, score);
}

bool threaded_node::simulate()
{
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

    //we need to make copies here because expand will modify the argument variables
    list<pos_move> our_hist = t_our.get_history();
    list<pos_move> opp_hist = t_opp.get_history();

    if ((our_hist.empty() && my_turn) || (opp_hist.empty() && !my_turn)) {//can't expand the tree if the current player can't move
        value_mutex.lock();
        visits++;
        scores += result;
        value_mutex.unlock();
        return false;
    } else {
        expand(our_hist, opp_hist, result);//this very node is definitely the parental node
        return true;
    }
}
