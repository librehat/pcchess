#include "board.h"
#include <stdexcept>
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

p_piece* board::operator [](const int &file)
{
    if (file >= FILE_NUM) {
        throw std::out_of_range("board file index out of range");
    }
    return data + file * RANK_NUM;
}

p_piece& board::operator [] (const position &pos)
{
    int loc = pos.file * RANK_NUM + pos.rank;
    if (loc >= NUM) {
        throw std::out_of_range("board position index out of range");
    }
    return data[pos.file * RANK_NUM + pos.rank];
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
