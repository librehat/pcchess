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

string pos_move::get_iccs() const
{
    std::string iccs;
    iccs.resize(4);
    iccs[0] = static_cast<char>(from.file + 'a');
    iccs[1] = static_cast<char>(from.rank + '0');
    iccs[2] = static_cast<char>(to.file + 'a');
    iccs[3] = static_cast<char>(to.rank + '0');
    return iccs;
}
