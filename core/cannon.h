/*
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef CANNON_H
#define CANNON_H

#include "abstract_piece.h"

class cannon : public abstract_piece
{
public:
    cannon(int _file, int _rank, bool oppo, board &_board) : abstract_piece(_file, _rank, oppo, _board) {}
    cannon(const cannon &b, board &new_board) : abstract_piece(b, new_board) {}

    abstract_piece* make_copy_with_new_board(board &bd) const;

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves();

private:
    void search_moves(const bool forward, const bool is_rank);
};

#endif //CANNON_H
