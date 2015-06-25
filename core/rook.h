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
    rook(int _file, int _rank, bool oppo) : abstract_piece(_file, _rank, oppo) {}

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves(board &bd);

private:
    void search_moves(const bool forward, const bool is_rank, board &b);
};

#endif //ROOK_H
