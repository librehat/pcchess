#ifndef NODE_H
#define NODE_H

#include <list>
#include "position.h"
#include "abstract_player.h"

class node
{
public:
    node(const abstract_player* _our, const abstract_player* _opp, bool _my_turn, node *_parent = nullptr);
    ~node();

    int get_visits() const;
    int get_scores() const;
    double get_value() const;
    const pos_move& get_our_move() const;
    const pos_move& get_opp_move() const;
    double get_uct_val() const;

    void set_our_move(const pos_move &m);
    void set_opp_move(const pos_move &m);

    //three steps for MCTS
    void select();
    void expand(std::list<pos_move> &our_hist, std::list<pos_move> &opp_hist, const int &score);
    int simulate();

    /*
     * select child according to the visit times
     * be aware of the return value could be nullptr (if children is empty)
     */
    node* get_best_child() const;

    void backpropagate(const int &score);
    void detach();

protected:
    bool my_turn;

    node* parent;
    std::list<node *> children;

    const abstract_player* our;
    const abstract_player* opp;

    pos_move our_move;
    pos_move opp_move;

    int visits;
    int scores;//the sum of simulation result where win: +1 draw: 0 lose: -1

    //find the children with same moves. return nullptr if no such child
    node* find_child(const pos_move &m);

    static const double uct_constant;
};

#endif // NODE_H
