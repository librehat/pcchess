/*
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef CANNON_H
#define CANNON_H

#include "abstract_piece.h"

class cannon : public abstract_piece
{
public:
    cannon() : abstract_piece() {}
    cannon(int _file, int _rank) : abstract_piece(_file, _rank) {}

    char abbr_name() const;
    int value() const;
    void update_moves(const abstract_piece ***b);

private:
    void search_moves(const bool forward, const bool is_rank, const abstract_piece ***b);
};

#endif //CANNON_H
