#include "soldier.h"

using namespace std;

const position soldier::up = position(0, -1);
const position soldier::left = position(-1, 0);
const position soldier::right = position(1, 0);

abstract_piece* soldier::make_a_copy() const
{
    return new soldier(*this);
}

char soldier::abbr_name() const
{
    return 'S';
}

string soldier::chinese_name() const
{
    return string("卒");
}

int soldier::value() const
{
    return 2;
}

bool soldier::can_move_horizontally()
{
    if (m_opposite) {
        return pos.rank > 4;
    } else {
        return pos.rank < 5;
    }
}

void soldier::gen_moves(const board &m_board)
{
    if (can_move_horizontally()) {
        avail_moves.push_back(position(pos + left));
        avail_moves.push_back(position(pos + right));
    }
    if (m_opposite) {
        avail_moves.push_back(position(pos - up));
    }
    else {
        avail_moves.push_back(position(pos + up));
    }
    remove_invalid_moves(m_board);
}
