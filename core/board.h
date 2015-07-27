#ifndef BOARD_H
#define BOARD_H

#include "position.h"
#include <array>

class abstract_piece;

typedef abstract_piece* p_piece;//normal pointer to abstract_piece

class board
{
public:
    board();

    p_piece at(const std::int8_t &file, const std::int8_t &rank) const;
    p_piece at(const position &pos) const;

    p_piece* operator [] (const std::int8_t &file);
    p_piece& operator [] (const position &pos);

    bool operator !=(const board&);

    void print_out(bool chinese_char = false) const;//print current chess board into stdout

    static const std::int8_t RANK_NUM = 10;
    static const std::int8_t FILE_NUM = 9;
    static const std::int8_t NUM = FILE_NUM * RANK_NUM;

    static std::string get_full_width_letter(const char &);


    typedef std::array<p_piece, NUM>::iterator iterator;
    typedef std::array<p_piece, NUM>::const_iterator const_iterator;

    iterator begin() { return data.begin(); }
    const_iterator begin() const { return data.cbegin(); }
    iterator end() { return data.end(); }
    const_iterator end() const { return data.cend(); }

private:
    std::array<p_piece, NUM> data;
};

#endif // BOARD_H
