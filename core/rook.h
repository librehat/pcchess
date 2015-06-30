/*
 * Rook, chariot.
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ROOK_H
#define ROOK_H

#include "abstract_piece.h"

class rook : public abstract_piece
{
public:
    rook(int _file, int _rank, bool oppo, board &_board) : abstract_piece(_file, _rank, oppo, _board) {}
    rook(const rook &b, board &new_board) : abstract_piece(b, new_board) {}

    abstract_piece* make_copy_with_new_board(board &bd) const;

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves();

private:
    void search_moves(const bool forward, const bool is_rank);
};

#endif //ROOK_H
