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

p_piece* board::operator [](const int8_t &file)
{
    if (file >= FILE_NUM || file < 0) {
        throw std::out_of_range("board file index out of range");
    }
    return data + file * RANK_NUM;
}

p_piece& board::operator [] (const position &pos)
{
    if (pos.not_in_range(0, FILE_NUM - 1, 0, RANK_NUM - 1)) {
        throw std::out_of_range("board position index out of range");
    }
    return data[pos.file * RANK_NUM + pos.rank];
}

p_piece board::at(const int8_t &file, const int8_t &rank) const
{
    if (file < 0 || file >= FILE_NUM || rank < 0 || rank >= RANK_NUM) {
        return nullptr;
    }

    return data[file * RANK_NUM + rank];
}

p_piece board::at(const position &pos) const
{
    return this->at(pos.file, pos.rank);
}

bool board::operator !=(const board &b)
{
    return data != b.data;
}
