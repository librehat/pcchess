#include "soldier.h"

const position soldier::up = position(0, -1);
const position soldier::left = position(-1, 0);
const position soldier::right = position(1, 0);

char soldier::abbr_name() const
{
    return 'S';
}

int soldier::value() const
{
    return 2;
}

void soldier::gen_moves(const abstract_piece***) 
{
    if (pos.rank < 5) {//in the opponent area
        avail_moves.push_back(position(pos + left));
        avail_moves.push_back(position(pos + right));
    }
    avail_moves.push_back(position(pos + up));
    remove_invalid_moves();
}
