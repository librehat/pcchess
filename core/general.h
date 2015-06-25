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
    general() : abstract_piece() {}
    general(int _file, int _rank, bool oppo) : abstract_piece(_file, _rank, oppo) {}

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves(board &b);

private:
    static const position up;
    static const position down;
    static const position left;
    static const position right;
};

#endif //GENERAL_H
