#ifndef BOARD_H
#define BOARD_H

#include "position.h"
#include <stdexcept>

class abstract_piece;

typedef abstract_piece* p_piece;//normal pointer to abstract_piece

class board
{
public:
    board();
    ~board();

    p_piece at(const int &file, const int &rank) const;
    p_piece at(const position &pos) const;

    inline p_piece* operator [] (const int &file)
    {
        if (file >= FILE_NUM) {
            throw std::out_of_range("board file index out of range");
        }
        return data + file * RANK_NUM;
    }

    inline p_piece& operator [] (const position &pos)
    {
        int loc = pos.file * RANK_NUM + pos.rank;
        if (loc >= NUM) {
            throw std::out_of_range("board position index out of range");
        }
        return data[pos.file * RANK_NUM + pos.rank];
    }

    bool operator !=(const board&);

    static const int RANK_NUM = 10;
    static const int FILE_NUM = 9;
    static const int NUM = FILE_NUM * RANK_NUM;

private:
    p_piece* data;
};

#endif // BOARD_H
