/*
 * pawn, 卒 or 兵. Also translated as soldier, private.
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef PAWN_H
#define PAWN_H

#include "abstract_piece.h"

class pawn : public abstract_piece
{
public:
    explicit pawn(std::int8_t _file = 0, std::int8_t _rank = 0, bool oppo = false) : abstract_piece(_file, _rank, oppo) {}
    explicit pawn(const pawn &b) : abstract_piece(b) {}

    abstract_piece* make_a_copy() const;

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves(const board &m_board);

private:
    bool can_move_horizontally();
    static const position up;
    static const position left;
    static const position right;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(abstract_piece);
    }
};

#endif //PAWN_H
