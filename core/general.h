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
    general(int _file, int _rank) : abstract_piece(_file, _rank) {}

    char abbr_name() const;
    int value() const;
};

#endif //GENERAL_H
