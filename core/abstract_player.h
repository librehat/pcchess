#ifndef ABSTRACT_PlAYER_H
#define ABSTRACT_PlAYER_H

#include "abstract_piece.h"
#include "position.h"
#include <list>

class abstract_player
{
public:
    abstract_player();
    virtual ~abstract_player();

    void add(abstract_piece *p);
    void remove(abstract_piece *p);

    /*
     * sub-class has to implement this pure virtual function
     * return true if a move is proposed and store the results
     * in arguments.
     * otherwise, return false if no move can be made
     */
    virtual bool think_next_move(position *from, position *to) = 0;

protected:
    std::list<abstract_piece*> pieces;
};

#endif //ABSTRACT_PlAYER_H
