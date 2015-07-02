/*
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ELEPHANT_H
#define ELEPHANT_H

#include "abstract_piece.h"

class elephant : public abstract_piece
{
public:
    elephant(int _file, int _rank, bool oppo) : abstract_piece(_file, _rank, oppo) {}
    elephant(const elephant &b) : abstract_piece(b) {}

    abstract_piece* make_a_copy() const;

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves(const board &m_board);

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
