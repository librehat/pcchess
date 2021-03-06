/*
 * King, also known as king in some translations.
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef KING_H
#define KING_H

#include "abstract_piece.h"

class king : public abstract_piece
{
public:
    explicit king(std::int8_t _file = 0, std::int8_t _rank = 0, bool oppo = false) : abstract_piece(_file, _rank, oppo) {}
    explicit king(const king &b) : abstract_piece(b) {}

    abstract_piece* make_a_copy() const;

    char abbr_name() const;
    std::string print_name(const bool &chinese = false) const;
    int value() const;
    bool is_king() const { return true; }
    
protected:
    void gen_moves(const board &m_board);

private:
    static const position up;
    static const position down;
    static const position left;
    static const position right;

    bool is_flying_king(const position &, const board &) const;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(abstract_piece);
    }
};

#endif //KING_H
