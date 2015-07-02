/*
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ADVISOR_H
#define ADVISOR_H

#include "abstract_piece.h"

class advisor : public abstract_piece
{
public:
    advisor(int _file, int _rank, bool oppo) : abstract_piece(_file, _rank, oppo) {}
    advisor(const advisor &b) : abstract_piece(b) {}

    abstract_piece* make_a_copy() const;

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves(const board &m_board);

private:
    static const position upleft;
    static const position upright;
    static const position downleft;
    static const position downright;
};

#endif //ADVISOR_H
