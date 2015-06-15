#include "abstract_piece.h"

abstract_piece::abstract_piece() :
    abstract_piece(0, 0)
{}

abstract_piece::abstract_piece(int _file, int _rank) :
    file(_file),
    rank(_rank)
{}

void abstract_piece::move_to_pos(int newfile, int newrank)
{
    file = newfile;
    rank = newrank;
}
