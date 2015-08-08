#include "chariot.h"

using namespace std;

abstract_piece* chariot::make_a_copy() const
{
    return new chariot(*this);
}

char chariot::abbr_name() const
{
    return red_side ? 'R' : 'r';
}

string chariot::print_name(const bool &chinese) const
{
    return red_side ? chinese ? string("俥") : string("Ｒ")
                    : chinese ? string("車") : string("ｒ");
}


int chariot::value() const
{
    return 6;
}

void chariot::search_moves(const bool forward, const bool is_rank, const board &m_board)
{
    for (position p = pos; !p.not_in_range(0, 8, 0, 9);
         is_rank ? (p.rank += forward ? 1 : -1) : (p.file += forward ? 1 : -1)
    ) {
        if (p == pos)   continue;
        avail_pos.push_back(p);
        if(m_board.at(p))   break;
    }
}

void chariot::gen_moves(const board &m_board)
{
    search_moves(true, true, m_board);
    search_moves(true, false, m_board);
    search_moves(false, true, m_board);
    search_moves(false, false, m_board);
    remove_invalid_moves(m_board);
}
