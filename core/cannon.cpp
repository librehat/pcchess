#include "cannon.h"

using namespace std;

char cannon::abbr_name() const
{
    return 'C';
}

string cannon::chinese_name() const
{
    return string("炮");
}

int cannon::value() const
{
    return 4;
}

void cannon::search_moves(const bool forward, const bool is_rank, board &b)
{
    bool mount_found = false;
    for (position p = pos; !p.not_in_range(0, 8, 0, 9);
         is_rank ? (p.rank += forward ? 1 : -1) : (p.file += forward ? 1 : -1)
    ) {
        if (p == pos)   continue;
        if(b[p]) {//a piece in the way
            if (mount_found) {//a piece that can be taken
                avail_moves.push_back(p);
                break;//cannon can't go further
            } else {
                mount_found = true;
            }
        } else {
            if (!mount_found) {//cannon can only move freely before the mount
                avail_moves.push_back(p);
            }
            //else simply continue path searching
        }
    }
}

void cannon::gen_moves(board &bd)
{
    search_moves(true, true, bd);
    search_moves(true, false, bd);
    search_moves(false, true, bd);
    search_moves(false, false, bd);
    remove_invalid_moves();//TODO this looks unnecessary for cannon
}
