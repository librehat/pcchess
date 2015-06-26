#include "elephant.h"

using namespace std;

const position elephant::up_left = position(-2, -2);
const position elephant::up_right = position(2, -2);
const position elephant::down_left = position(-2, 2);
const position elephant::down_right = position(2, 2);

char elephant::abbr_name() const
{
    return 'E';
}

string elephant::chinese_name() const
{
    return string("象");
}

int elephant::value() const
{
    return 2;
}

void elephant::gen_moves()
{
    avail_moves.push_back(pos + up_left);
    avail_moves.push_back(pos + up_right);
    avail_moves.push_back(pos + down_left);
    avail_moves.push_back(pos + down_right);
    remove_invalid_moves(0, 8, m_opposite ? 0 : 5, m_opposite ? 4 : 9);//elephant can't across the river
}
