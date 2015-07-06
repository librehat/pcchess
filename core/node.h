#ifndef NODE_H
#define NODE_H

#include <list>
#include "position.h"
#include "abstract_player.h"

class node
{
public:
    //WARN: the node will take memory control of _our and _opp pointers!
    node(abstract_player* _our, abstract_player* _opp, bool _my_turn, node *_parent = nullptr);
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
    bool select();//return true if it did a successful simulation
    void expand(std::list<pos_move> &our_hist, std::list<pos_move> &opp_hist, const int &score);
    bool simulate();

    /*
     * select child according to the visit times
     * be aware of the return value could be nullptr (if children is empty)
     */
    node* get_best_child() const;//best child which has highest visits
    node* get_best_child_uct() const;//return best child which has maximum value of get_uct_val()

    void remoev_child(node *);

    void backpropagate(const int &score);
    void detach();

    /*
     * find the children with same moves (our_move or opp_move, up to the parental my_turn value).
     * return nullptr if no such child
     */
    node* find_child(const pos_move &m);

protected:
    bool my_turn;

    node* parent;
    std::list<node *> children;

    //"current": the state in this node
    abstract_player* our_curr;
    abstract_player* opp_curr;

    pos_move our_move;
    pos_move opp_move;

    int visits;
    int scores;//the sum of simulation result where win: +1 draw: 0 lose: -1

    static const int select_threshold;
    static const double uct_constant;
};

#endif // NODE_H
