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
    cannon(int _file, int _rank, bool oppo) : abstract_piece(_file, _rank, oppo) {}

    char abbr_name() const;
    int value() const;
    
protected:
    void gen_moves(abstract_piece ***b);

private:
    void search_moves(const bool forward, const bool is_rank, abstract_piece ***b);
};

#endif //CANNON_H
