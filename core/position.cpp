#include "position.h"

using namespace std;

position::position(int8_t _file, int8_t _rank) :
    file(_file),
    rank(_rank)
{}

position::position(const position &b)
{
    *this = b;
}

bool position::not_in_range(int8_t min_file, int8_t max_file, int8_t min_rank, int8_t max_rank) const
{
    return file < min_file || file > max_file || rank < min_rank || rank > max_rank;
}

position& position::operator =(const position &b)
{
    if (this != &b) {//self-assignment check
        this->file = b.file;
        this->rank = b.rank;
    }
    return *this;
}

position position::operator +(const position &b)
{
    return position(this->file + b.file, this->rank + b.rank);
}

position position::operator -(const position &b)
{
    return position(this->file - b.file, this->rank - b.rank);
}

bool position::operator <(const position &b) const
{
    if (file < b.file) {
        return true;
    } else if (file == b.file) {
        return rank < b.rank;
    } else {
        return false;
    }
}
