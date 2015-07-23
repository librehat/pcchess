#include "pawn.h"

using namespace std;

const position pawn::up = position(0, -1);
const position pawn::left = position(-1, 0);
const position pawn::right = position(1, 0);

abstract_piece* pawn::make_a_copy() const
{
    return new pawn(*this);
}

char pawn::abbr_name() const
{
    return 'P';//pawn
}

string pawn::chinese_name() const
{
    return string("卒");
}

int pawn::value() const
{
    return 2;
}

bool pawn::can_move_horizontally()
{
    if (m_opposite) {
        return pos.rank > 4;
    } else {
        return pos.rank < 5;
    }
}

void pawn::gen_moves(const board &m_board)
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
