#include "rook.h"

using namespace std;

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

void rook::search_moves(const bool forward, const bool is_rank, board &b)
{
    for (position p = pos; !p.not_in_range(0, 8, 0, 9);
         is_rank ? (p.rank += forward ? 1 : -1) : (p.file += forward ? 1 : -1)
    ) {
        if (p == pos)   continue;
        avail_moves.push_back(p);
        if(b[p])   break;
    }
}

void rook::gen_moves(board &bd)
{
    search_moves(true, true, bd);
    search_moves(true, false, bd);
    search_moves(false, true, bd);
    search_moves(false, false, bd);
    remove_invalid_moves();
}
