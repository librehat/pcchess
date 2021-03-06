/*
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ELEPHANT_H
#define ELEPHANT_H

#include "abstract_piece.h"

class elephant : public abstract_piece
{
public:
    explicit elephant(std::int8_t _file = 0, std::int8_t _rank = 0, bool oppo = false) : abstract_piece(_file, _rank, oppo) {}
    explicit elephant(const elephant &b) : abstract_piece(b) {}

    abstract_piece* make_a_copy() const;

    char abbr_name() const;
    std::string print_name(const bool &chinese = false) const;
    int value() const;
    
protected:
    void gen_moves(const board &m_board);

private:
    static const position up_left;
    static const position up_right;
    static const position down_left;
    static const position down_right;

    static const position up_left_centre;
    static const position up_right_centre;
    static const position down_left_centre;
    static const position down_right_centre;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(abstract_piece);
    }
};

#endif //ELEPHANT_H
