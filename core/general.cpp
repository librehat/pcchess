#include "general.h"

const position general::up = position(0, -1);
const position general::down = position(0, 1);
const position general::left = position(-1, 0);
const position general::right = position(1, 0);

char general::abbr_name() const
{
    return 'G';
}

int general::value() const
{
    return 8;
}

void general::gen_moves(const abstract_piece***)
{
    avail_moves.push_back(pos + up);
    avail_moves.push_back(pos + down);
    avail_moves.push_back(pos + left);
    avail_moves.push_back(pos + right);
    remove_invalid_moves(3, 5, 7, 9);
}
