#ifndef NODE_H
#define NODE_H

#include <list>
#include "position.h"
#include "board.h"
#include "abstract_player.h"

class node
{
public:
    node(node *_parent);
    node(const abstract_player* _our, const abstract_player* _opp);//this is only for root node
    ~node();

    int get_visits() const;
    int get_scores() const;

    /*
     * select child according to the visit times
     * be aware of the return value could be nullptr (if children is empty)
     */
    node* get_best_child() const;

    void play_random_game();
    void backpropagate(const int &score);
    void detach();

protected:
    node* parent;
    std::list<node *> children;

    board t_board;
    abstract_player* our;
    abstract_player* opp;

    pos_move m_move;

    int visits;
    int scores;
};

#endif // NODE_H
