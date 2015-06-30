/*
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ELEPHANT_H
#define ELEPHANT_H

#include "abstract_piece.h"

class elephant : public abstract_piece
{
public:
    elephant(int _file, int _rank, bool oppo, board &_board) : abstract_piece(_file, _rank, oppo, _board) {}
    elephant(const elephant &b, board &new_board) : abstract_piece(b, new_board) {}

    abstract_piece* make_copy_with_new_board(board &bd) const;

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves();

private:
    static const position up_left;
    static const position up_right;
    static const position down_left;
    static const position down_right;

    static const position up_left_centre;
    static const position up_right_centre;
    static const position down_left_centre;
    static const position down_right_centre;
};

#endif //ELEPHANT_H
