#include "rook.h"

char rook::abbr_name() const
{
    return 'R';
}

int rook::value() const
{
    return 6;
}

void rook::search_moves(const bool forward, const bool is_rank, abstract_piece ***b)
{
    for (position p = pos; !p.not_in_range(0, 8, 0, 9);
         is_rank ? (p.rank += forward ? 1 : -1) : (p.file += forward ? 1 : -1)
    ) {
        if (p == pos)   continue;
        avail_moves.push_back(p);
        if(b[p.file][p.rank])   break;
    }
}

void rook::gen_moves(abstract_piece*** b)
{
    search_moves(true, true, b);
    search_moves(true, false, b);
    search_moves(false, true, b);
    search_moves(false, false, b);
    remove_invalid_moves();
}
