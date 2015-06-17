/*
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ELEPHANT_H
#define ELEPHANT_H

#include "abstract_piece.h"

class elephant : public abstract_piece
{
public:
    elephant() : abstract_piece() {}
    elephant(int _file, int _rank) : abstract_piece(_file, _rank) {}

    char abbr_name() const;
    int value() const;
    void update_moves(const abstract_piece*** b);

private:
    static const position up_left;
    static const position up_right;
    static const position down_left;
    static const position down_right;
};

#endif //ELEPHANT_H
