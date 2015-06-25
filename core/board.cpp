#include "board.h"
#include <cstring>
#include <stdexcept>

using namespace std;

board::board()
{
    data = new p_piece[90];

    for (int i = 0; i < 90; i++) {
        data[i] = nullptr;
    }
}

board::board(const board &b)
{
    *this = b;
}

board::~board()
{
    delete [] data;
}

p_piece board::at(const int &file, const int &rank) const
{
    int dis = file * 10 + rank;
    if (dis >= 90) {
        throw runtime_error("Error. Accessing an element in board out of range.");
    }
    return data[dis];
}

board &board::operator = (const board &b)
{
    memcpy(data, b.data, 90 * sizeof(p_piece));
    return *this;
}
