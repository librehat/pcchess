/*
 * horse, 马. Also translated as knight.
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef HORSE_H
#define HORSE_H

#include "abstract_piece.h"

class horse : public abstract_piece
{
public:
    explicit horse(int _file = 0, int _rank = 0, bool oppo = false) : abstract_piece(_file, _rank, oppo) {}
    explicit horse(const horse &b) : abstract_piece(b) {}

    abstract_piece* make_a_copy() const;


    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves(const board &m_board);

private:
    //go left/right and up/down
    static const position l2u1;
    static const position l1u2;
    static const position r2u1;
    static const position r1u2;
    static const position l2d1;
    static const position l1d2;
    static const position r2d1;
    static const position r1d2;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(abstract_piece);
    }
};

#endif //HORSE_H
