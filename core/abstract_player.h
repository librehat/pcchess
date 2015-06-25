#ifndef ABSTRACT_PlAYER_H
#define ABSTRACT_PlAYER_H

#include "abstract_piece.h"
#include "position.h"
#include <list>

class abstract_player
{
public:
    abstract_player(board &bd);
    virtual ~abstract_player();

    void add(p_piece p);
    void remove(p_piece p);

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
    virtual bool think_next_move(position *from, position *to) = 0;

protected:
    std::list<p_piece> pieces;
    bool checked;//if our general is checked
    bool checkmated;//if our general is dead
    board &m_board;
};

#endif //ABSTRACT_PlAYER_H
