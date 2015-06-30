#include "elephant.h"

using namespace std;

const position elephant::up_left = position(-2, -2);
const position elephant::up_right = position(2, -2);
const position elephant::down_left = position(-2, 2);
const position elephant::down_right = position(2, 2);

const position elephant::up_left_centre = position(-1, -1);
const position elephant::up_right_centre = position(1, -1);
const position elephant::down_left_centre = position(-1, 1);
const position elephant::down_right_centre = position(1, 1);

abstract_piece* elephant::make_copy_with_new_board(board &bd) const
{
	return new elephant(*this, bd);
}

char elephant::abbr_name() const
{
    return 'E';
}

string elephant::chinese_name() const
{
    return string("象");
}

int elephant::value() const
{
    return 2;
}

void elephant::gen_moves()
{
    if (!m_board[pos + up_left_centre]) {
        avail_moves.push_back(pos + up_left);
    }
    if (!m_board[pos + up_right_centre]) {
        avail_moves.push_back(pos + up_right);
    }
    if (!m_board[pos + down_left_centre]) {
        avail_moves.push_back(pos + down_left);
    }
    if (!m_board[pos + down_right_centre]) {
        avail_moves.push_back(pos + down_right);
    }
    remove_invalid_moves(0, 8, m_opposite ? 0 : 5, m_opposite ? 4 : 9);//elephant can't across the river
}
