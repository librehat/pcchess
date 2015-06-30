/*
 * General, also known as King in some translations.
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef GENERAL_H
#define GENERAL_H

#include "abstract_piece.h"

class general : public abstract_piece
{
public:
    general(int _file, int _rank, bool oppo, board &_board) : abstract_piece(_file, _rank, oppo, _board) {}
    general(const general &b, board &new_board) : abstract_piece(b, new_board) {}

    abstract_piece* make_copy_with_new_board(board &bd) const;

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves();

private:
    static const position up;
    static const position down;
    static const position left;
    static const position right;
};

#endif //GENERAL_H
