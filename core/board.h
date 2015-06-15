/*
 * The chess board class
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef BOARD_H
#define BOARD_H

#include "abstract_piece.h"

class board
{
public:
    board();
    ~board();

private:
    /*
     * this is NOT a 2D array's pointer
     * because we use pointers on the board to indicate
     * if a position is occupied by a piece
     */
    abstract_piece** data;
};

#endif //BOARD_H
