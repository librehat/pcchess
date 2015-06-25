/*
 * soldier, 卒 or 兵. Also translated as pawn, private.
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef SOLDIER_H
#define SOLDIER_H

#include "abstract_piece.h"

class soldier : public abstract_piece
{
public:
    soldier() : abstract_piece() {}
    soldier(int _file, int _rank, bool oppo) : abstract_piece(_file, _rank, oppo) {}

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves(board &);

private:
    bool can_move_horizontally();
    static const position up;
    static const position left;
    static const position right;
};

#endif //SOLDIER_H
