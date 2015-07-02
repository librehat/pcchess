#include "rook.h"

using namespace std;

abstract_piece* rook::make_a_copy() const
{
    return new rook(*this);
}

char rook::abbr_name() const
{
    return 'R';
}

string rook::chinese_name() const
{
    return string("車");
}

int rook::value() const
{
    return 6;
}

void rook::search_moves(const bool forward, const bool is_rank, const board &m_board)
{
    for (position p = pos; !p.not_in_range(0, 8, 0, 9);
         is_rank ? (p.rank += forward ? 1 : -1) : (p.file += forward ? 1 : -1)
    ) {
        if (p == pos)   continue;
        avail_moves.push_back(p);
        if(m_board.at(p))   break;
    }
}

void rook::gen_moves(const board &m_board)
{
    search_moves(true, true, m_board);
    search_moves(true, false, m_board);
    search_moves(false, true, m_board);
    search_moves(false, false, m_board);
    remove_invalid_moves(m_board);
}
