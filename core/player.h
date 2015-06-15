/*
 * Player class
 * 
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <array>
#include <list>
#include "abstract_piece.h"

class player
{
public:
    player();
    ~player();

private:
    /*
     * each side has at most 16 pieces in total
     * this class manages their memories
     * piece_list only contains **alive** pieces
     * a piece will be deleted and released if it's dead
     */
    std::list<abstract_piece*> piece_list;
};

#endif //PLAYER_H
