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
    horse() : abstract_piece() {}
    horse(int _file, int _rank) : abstract_piece(_file, _rank) {}

    char abbr_name() const;
    int value() const;
};

#endif //HORSE_H
