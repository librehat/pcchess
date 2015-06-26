/*
 * horse, 马. Also translated as knight.
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef HORSE_H
#define HORSE_H

#include "abstract_piece.h"

class horse : public abstract_piece
{
public:
    horse(int _file, int _rank, bool oppo, board &_board) : abstract_piece(_file, _rank, oppo, _board) {}

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves();

private:
    //go left/right and up/down
    static const position l2u1;
    static const position l1u2;
    static const position r2u1;
    static const position r1u2;
    static const position l2d1;
    static const position l1d2;
    static const position r2d1;
    static const position r1d2;
};

#endif //HORSE_H
