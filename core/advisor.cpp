#include "advisor.h"

using namespace std;

const position advisor::upleft = position(-1, 1);
const position advisor::upright = position(1, 1);
const position advisor::downleft = position(-1, -1);
const position advisor::downright = position(1, -1);

abstract_piece* advisor::make_copy_with_new_board(board &bd) const
{
	return new advisor(*this, bd);
}

char advisor::abbr_name() const
{
     return 'A';
}

string advisor::chinese_name() const
{
    return string("士");
}

int advisor::value() const
{
    return 2;
}

void advisor::gen_moves()
{
    avail_moves.push_back(pos + upleft);
    avail_moves.push_back(pos + upright);
    avail_moves.push_back(pos + downleft);
    avail_moves.push_back(pos + downright);
    remove_invalid_moves(3, 5, m_opposite ? 0 : 7, m_opposite ? 2 : 9);
}

