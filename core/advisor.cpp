#include "advisor.h"

using namespace std;

const position advisor::upleft = position(-1, 1);
const position advisor::upright = position(1, 1);
const position advisor::downleft = position(-1, -1);
const position advisor::downright = position(1, -1);

abstract_piece* advisor::make_a_copy() const
{
    return new advisor(*this);
}

char advisor::abbr_name() const
{
     return red_side ? 'A' : 'a';
}

string advisor::print_name(const bool &chinese) const
{
    return red_side ? chinese ? string("仕") : string("Ａ")
                    : chinese ? string("士") : string("ａ");
}

int advisor::value() const
{
    return 2;
}

void advisor::gen_moves(const board &m_board)
{
    avail_pos.push_back(pos + upleft);
    avail_pos.push_back(pos + upright);
    avail_pos.push_back(pos + downleft);
    avail_pos.push_back(pos + downright);
    remove_invalid_moves(m_board, 3, 5, red_side ? 0 : 7, red_side ? 2 : 9);
}

