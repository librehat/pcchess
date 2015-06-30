#include "rook.h"

using namespace std;

abstract_piece* rook::make_copy_with_new_board(board &bd) const
{
	return new rook(*this, bd);
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

void rook::search_moves(const bool forward, const bool is_rank)
{
    for (position p = pos; !p.not_in_range(0, 8, 0, 9);
         is_rank ? (p.rank += forward ? 1 : -1) : (p.file += forward ? 1 : -1)
    ) {
        if (p == pos)   continue;
        avail_moves.push_back(p);
        if(m_board[p])   break;
    }
}

void rook::gen_moves()
{
    search_moves(true, true);
    search_moves(true, false);
    search_moves(false, true);
    search_moves(false, false);
    remove_invalid_moves();
}
