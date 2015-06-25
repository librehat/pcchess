#ifndef BOARD_H
#define BOARD_H

#include "position.h"

class abstract_piece;

typedef abstract_piece* p_piece;//pointer to abstract_piece

class board
{
public:
    board();
    ~board();

    p_piece** &data_ref();

    p_piece at(const int &file, const int &rank) const;

    inline p_piece* &operator [] (const int &file)
    {
        return data[file];
    }

    inline p_piece  &operator [] (const position &pos)
    {
        return data_2[pos.file * 10 + pos.rank];
    }

private:
    p_piece** data;
    p_piece*  data_2;
};

#endif // BOARD_H
