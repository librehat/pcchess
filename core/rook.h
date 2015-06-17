/*
 * Rook, chariot.
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ROOK_H
#define ROOK_H

#include "abstract_piece.h"

class rook : public abstract_piece
{
public:
    rook() : abstract_piece() {}
    rook(int _file, int _rank) : abstract_piece(_file, _rank) {}

    char abbr_name() const;
    int value() const;
    void update_moves(const abstract_piece*** b);

private:
    void search_moves(const bool forward, const bool is_rank, const abstract_piece ***b);
};

#endif //ROOK_H
