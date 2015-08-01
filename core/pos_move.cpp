#include "pos_move.h"

using namespace std;

pos_move::pos_move(const position &f, const position &t) :
    from (f),
    to (t)
{}

pos_move::pos_move(const string &iccs)
{
    from.file = static_cast<int8_t>(iccs[0] - 'a');
    from.rank = static_cast<int8_t>(iccs[1] - '0');
    to.file = static_cast<int8_t>(iccs[2] - 'a');
    to.rank = static_cast<int8_t>(iccs[3] - '0');
}

bool pos_move::is_valid() const
{
    return from.file != 0 || from.rank != 0 || to.file != 0 || to.rank != 0;
}

string pos_move::get_iccs() const
{
    ostringstream stream;
    stream << from << to;
    return stream.str();
}
