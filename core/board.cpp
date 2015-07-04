#include "board.h"
#include <stdexcept>
#include <iostream>

using namespace std;

board::board()
{
    data = new p_piece[90];

    for (int i = 0; i < 90; i++) {
        data[i] = nullptr;
    }
}

board::~board()
{
    delete [] data;
}

p_piece board::at(const int &file, const int &rank) const
{
    int dis = file * 10 + rank;
    if (dis >= 90) {
#ifdef _DEBUG
        cerr << "Trying to access file " << file << " rank " << rank << endl;
#endif
        throw runtime_error("Error. Accessing an element in board out of range.");
    }
    return data[dis];
}

p_piece board::at(const position &pos) const
{
    return this->at(pos.file, pos.rank);
}

bool board::operator !=(const board &b)
{
    return data != b.data;
}
