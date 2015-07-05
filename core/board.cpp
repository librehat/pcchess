#include "board.h"
#include <iostream>

using namespace std;

board::board()
{
    data = new p_piece[NUM];

    for (int i = 0; i < NUM; i++) {
        data[i] = nullptr;
    }
}

board::~board()
{
    delete [] data;
}

p_piece board::at(const int &file, const int &rank) const
{
    int dis = file * RANK_NUM + rank;
    if (dis >= NUM) {
#ifdef _DEBUG
        cerr << "Trying to access file " << file << " rank " << rank << endl;
#endif
        return nullptr;
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
