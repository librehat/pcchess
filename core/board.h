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

    p_piece at(const std::int8_t &file, const std::int8_t &rank) const;
    p_piece at(const position &pos) const;

    p_piece* operator [] (const std::int8_t &file);
    p_piece& operator [] (const position &pos);

    bool operator !=(const board&);

    static const std::int8_t RANK_NUM = 10;
    static const std::int8_t FILE_NUM = 9;
    static const std::int8_t NUM = FILE_NUM * RANK_NUM;

private:
    p_piece* data;
};

#endif // BOARD_H
