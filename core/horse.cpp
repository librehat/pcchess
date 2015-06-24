#include "horse.h"

using namespace std;

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

string horse::chinese_name() const
{
    return string("馬");
}

int horse::value() const
{
    return 4;
}

void horse::gen_moves(abstract_piece*** b)
{
    if(pos.file == 0 || !b[pos.file - 1][pos.rank]) {
        avail_moves.push_back(pos + l2u1);
        avail_moves.push_back(pos + l2d1);
    }
    if(pos.file == 8 || !b[pos.file + 1][pos.rank]) {
        avail_moves.push_back(pos + r2u1);
        avail_moves.push_back(pos + r2d1);
    }
    if(pos.rank == 0 || !b[pos.file][pos.rank - 1]) {
        avail_moves.push_back(pos + l1u2);
        avail_moves.push_back(pos + r1u2);
    }
    if(pos.rank == 9 || !b[pos.file][pos.rank + 1]) {
        avail_moves.push_back(pos + l1d2);
        avail_moves.push_back(pos + r1d2);
    }
    remove_invalid_moves();
}
