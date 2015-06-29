#ifndef BOARD_H
#define BOARD_H

#include "position.h"
#include <memory>

class abstract_piece;

typedef std::shared_ptr<abstract_piece> p_piece;//shared pointer to abstract_piece

class board
{
public:
    board();
    board(const board&);
    ~board();

    p_piece at(const int &file, const int &rank) const;
    board& operator =(const board&);

    inline p_piece* operator [] (const int &file)
    {
        return data + file * 10;
    }

    inline p_piece& operator [] (const position &pos)
    {
        return data[pos.file * 10 + pos.rank];
    }

    bool operator !=(const board&);

private:
    p_piece* data;
};

#endif // BOARD_H
