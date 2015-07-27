#include "elephant.h"

using namespace std;

const position elephant::up_left = position(-2, 2);
const position elephant::up_right = position(2, 2);
const position elephant::down_left = position(-2, -2);
const position elephant::down_right = position(2, -2);

const position elephant::up_left_centre = position(-1, 1);
const position elephant::up_right_centre = position(1, 1);
const position elephant::down_left_centre = position(-1, -1);
const position elephant::down_right_centre = position(1, -1);

abstract_piece* elephant::make_a_copy() const
{
    return new elephant(*this);
}

char elephant::abbr_name() const
{
    return red_side ? 'B' : 'b';//E in WXF document
}

string elephant::chinese_name() const
{
    return red_side ? string("相") : string("象");
}

int elephant::value() const
{
    return 2;
}

void elephant::gen_moves(const board &m_board)
{
    if (!m_board.at(pos + up_left_centre)) {
        avail_pos.push_back(pos + up_left);
    }
    if (!m_board.at(pos + up_right_centre)) {
        avail_pos.push_back(pos + up_right);
    }
    if (!m_board.at(pos + down_left_centre)) {
        avail_pos.push_back(pos + down_left);
    }
    if (!m_board.at(pos + down_right_centre)) {
        avail_pos.push_back(pos + down_right);
    }
    remove_invalid_moves(m_board, 0, 8, red_side ? 0 : 5, red_side ? 4 : 9);//elephant can't across the river
}
