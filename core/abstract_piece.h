/*
 * Abstract piece class
 *
 * This is the base class for all pieces
 * 
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ABSTRACT_PIECE_H
#define ABSTRACT_PIECE_H

#include "position.h"
#include <list>

class abstract_piece
{
public:
    abstract_piece();
    abstract_piece(int _file, int _rank);
    virtual ~abstract_piece();

    void move_to_pos(int newfile, int newrank);
    void move_to_pos(const position &new_pos);
    /*
     * re-generate avail_moves
     * give the board data to this function
     */
    virtual void update_moves(const abstract_piece*** board);
    const std::list<position>& get_avail_moves() const;

    virtual char abbr_name() const;
    virtual int value() const;//the "value" of this piece indicates how important it is

protected:
    void remove_invalid_moves(int min_file = 0, int max_file = 8, int min_rank = 0, int max_rank = 9);//invalid: out of the given scope, default to the board

    position pos;
    std::list<position> avail_moves;
};

#endif //ABSTRACT_PIECE_H
