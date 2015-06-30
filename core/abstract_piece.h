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
#include "board.h"
#include <list>
#include <string>

class abstract_piece
{
public:
    abstract_piece(int _file, int _rank, bool oppo, board &_board);
    abstract_piece(const abstract_piece &b, board &new_board);
    virtual ~abstract_piece();

    /*
     * if it's in opposite side, we may need to take care of the
     * coordinate system.
     */
    const bool is_opposite_side() const;

    position get_position() const;

    void move_to_pos(int newfile, int newrank);
    void move_to_pos(const position &new_pos);
    
    /*
     * re-generate avail_moves
     * give the board data to this function
     */
    void update_moves();
    const std::list<position>& get_avail_moves() const;
    bool is_movable() const;//check if avail_moves is empty

    virtual char abbr_name() const = 0;
    virtual std::string chinese_name() const = 0;
    virtual int value() const = 0;//the "value" of this piece indicates how important it is

    virtual abstract_piece* make_copy_with_new_board(board &bd) const = 0;

    bool operator ==(const abstract_piece &b);
    bool operator !=(const abstract_piece &b);

protected:
    board &m_board;
    const bool m_opposite;
    /*
     * a piece can't move if it's the only piece separate generals
     * return false if it's such a piece hence it can't move
     * return true if it can move freely
     */
    bool can_i_move() const;
    
    virtual void gen_moves() = 0;
    void remove_invalid_moves(int min_file = 0, int max_file = 8, int min_rank = 0, int max_rank = 9);//invalid: out of the given scope, default to the board

    position pos;
    std::list<position> avail_moves;
};

#endif //ABSTRACT_PIECE_H
