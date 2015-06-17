#include "abstract_piece.h"

using namespace std;

abstract_piece::abstract_piece() :
    abstract_piece(0, 0)
{}

abstract_piece::abstract_piece(int _file, int _rank) :
    pos(_file, _rank)
{}

void abstract_piece::move_to_pos(int newfile, int newrank)
{
    pos.file = newfile;
    pos.rank = newrank;
}

void abstract_piece::move_to_pos(const position &new_pos)
{
    pos = new_pos;
}

void abstract_piece::remove_invalid_moves(int min_file, int max_file, int min_rank, int max_rank)
{
    for (auto it = avail_moves.begin(); it != avail_moves.end();) {
        if (it->not_in_range(min_file, max_file, min_rank, max_rank)) {
            it = avail_moves.erase(it);
        } else {
            ++it;
        }
    }
}

const list<position>& abstract_piece::get_avail_moves() const
{
    return avail_moves;
}
