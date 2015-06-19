#include "horse.h"

const position horse::l2u1 = position(-2, -1);
const position horse::l1u2 = position(-1, -2);
const position horse::r2u1 = position( 2, -1);
const position horse::r1u2 = position( 1, -2);
const position horse::l2d1 = position(-2,  1);
const position horse::l1d2 = position(-1,  2);
const position horse::r2d1 = position( 2,  1);
const position horse::r1d2 = position( 1,  2);

char horse::abbr_name() const
{
    return 'H';
}

int horse::value() const
{
    return 4;
}

void horse::gen_moves(const abstract_piece*** b)
{
    if(!b[pos.file - 1][pos.rank]) {
        avail_moves.push_back(pos + l2u1);
        avail_moves.push_back(pos + l2d1);
    }
    if(!b[pos.file + 1][pos.rank]) {
        avail_moves.push_back(pos + r2u1);
        avail_moves.push_back(pos + r2d1);
    }
    if(!b[pos.file][pos.rank - 1]) {
        avail_moves.push_back(pos + l1u2);
        avail_moves.push_back(pos + r1u2);
    }
    if(!b[pos.file][pos.rank + 1]) {
        avail_moves.push_back(pos + l1d2);
        avail_moves.push_back(pos + r1d2);
    }
    remove_invalid_moves();
}
