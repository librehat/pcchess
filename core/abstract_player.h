#ifndef ABSTRACT_PlAYER_H
#define ABSTRACT_PlAYER_H

#include "abstract_piece.h"
#include "position.h"
#include <list>

class abstract_player
{
public:
    abstract_player();
    abstract_player(const abstract_player &b);
    virtual ~abstract_player();

    void add(p_piece p);
    bool remove(p_piece p);//return true if there is p in pieces and gets removed successfully.
    void add_history(const position &from, const position &to);
    void add_history(const pos_move &m);
    const std::list<pos_move>& get_history() const;
    void clear_history();

    /*
     * initialise the pieces list to a normal beginning situation
     * no need to call add function multiple times
     * opposite: whether this player is opposite player
     */
    void init_pieces(bool opposite = false);

    bool is_checked() const;
    bool is_checkmated() const;
    const std::list<p_piece> &get_pieces() const;

    /*
     * sub-class has to implement this pure virtual function
     * return true if a move is proposed and store the results
     * in arguments.
     * otherwise, return false if no move can be made
     */
    virtual bool think_next_move(pos_move &m, const board &bd) = 0;

    /*
     * the sub-class player might want to change some strategies
     * after the opponent moved.
     * this function does nothing by default
     */
    virtual void opponent_moved(const pos_move &) {}

protected:
    std::list<p_piece> pieces;
    std::list<pos_move> move_history;//front: latest move, back: the first move
    bool checked;//if our general is checked
    bool checkmated;//if our general is dead
};

#endif //ABSTRACT_PlAYER_H
