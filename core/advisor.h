/*
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ADVISOR_H
#define ADVISOR_H

#include "abstract_piece.h"

class advisor : public abstract_piece
{
public:
    advisor() : abstract_piece() {}
    advisor(int _file, int _rank, bool oppo) : abstract_piece(_file, _rank, oppo) {}

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves(abstract_piece***);

private:
    static const position upleft;
    static const position upright;
    static const position downleft;
    static const position downright;
};

#endif //ADVISOR_H
