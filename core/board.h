#ifndef BOARD_H
#define BOARD_H

#include "position.h"

class abstract_piece;

typedef abstract_piece* p_piece;//normal pointer to abstract_piece

class board
{
public:
    board();
    ~board();

    p_piece at(const int &file, const int &rank) const;
    p_piece at(const position &pos) const;

    p_piece* operator [] (const int &file);
    p_piece& operator [] (const position &pos);

    bool operator !=(const board&);

    static const int RANK_NUM = 10;
    static const int FILE_NUM = 9;
    static const int NUM = FILE_NUM * RANK_NUM;

private:
    p_piece* data;
};

#endif // BOARD_H
