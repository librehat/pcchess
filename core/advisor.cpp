#include "advisor.h"

const position advisor::upleft = position(-1, 1);
const position advisor::upright = position(1, 1);
const position advisor::downleft = position(-1, -1);
const position advisor::downright = position(1, -1);

char advisor::abbr_name() const
{
     return 'A';
}

int advisor::value() const
{
    return 2;
}

void advisor::gen_moves(const abstract_piece***)
{
    avail_moves.push_back(pos + upleft);
    avail_moves.push_back(pos + upright);
    avail_moves.push_back(pos + downleft);
    avail_moves.push_back(pos + downright);
    remove_invalid_moves(3, 5, 7, 9);
}

