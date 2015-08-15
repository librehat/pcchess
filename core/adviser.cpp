#include "adviser.h"

using namespace std;

const position adviser::upleft = position(-1, 1);
const position adviser::upright = position(1, 1);
const position adviser::downleft = position(-1, -1);
const position adviser::downright = position(1, -1);

abstract_piece* adviser::make_a_copy() const
{
    return new adviser(*this);
}

char adviser::abbr_name() const
{
     return red_side ? 'A' : 'a';
}

string adviser::print_name(const bool &chinese) const
{
    return red_side ? chinese ? string("仕") : string("Ａ")
                    : chinese ? string("士") : string("ａ");
}

int adviser::value() const
{
    return 2;
}

void adviser::gen_moves(const board &m_board)
{
    avail_pos.push_back(pos + upleft);
    avail_pos.push_back(pos + upright);
    avail_pos.push_back(pos + downleft);
    avail_pos.push_back(pos + downright);
    remove_invalid_moves(m_board, 3, 5, red_side ? 0 : 7, red_side ? 2 : 9);
}

