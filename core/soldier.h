/*
 * soldier, 卒 or 兵. Also translated as pawn, private.
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef SOLDIER_H
#define SOLDIER_H

#include "abstract_piece.h"

class soldier : public abstract_piece
{
public:
    soldier(int _file = 0, int _rank = 0, bool oppo = false) : abstract_piece(_file, _rank, oppo) {}
    soldier(const soldier &b) : abstract_piece(b) {}

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
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(abstract_piece);
    }
};

#endif //SOLDIER_H
