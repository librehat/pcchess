#include "cannon.h"

using namespace std;

abstract_piece* cannon::make_a_copy() const
{
    return new cannon(*this);
}

char cannon::abbr_name() const
{
    return red_side ? 'C' : 'c';
}

string cannon::chinese_name() const
{
    return red_side ? string("炮") : string("砲");
}

int cannon::value() const
{
    return 4;
}

void cannon::search_moves(const bool forward, const bool is_rank, const board &m_board)
{
    bool mount_found = false;
    for (position p = pos; !p.not_in_range(0, 8, 0, 9);
         is_rank ? (p.rank += forward ? 1 : -1) : (p.file += forward ? 1 : -1)
    ) {
        if (p == pos)   continue;
        if(m_board.at(p)) {//a piece in the way
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

void cannon::gen_moves(const board &m_board)
{
    search_moves(true, true, m_board);
    search_moves(true, false, m_board);
    search_moves(false, true, m_board);
    search_moves(false, false, m_board);
    remove_invalid_moves(m_board);
}
