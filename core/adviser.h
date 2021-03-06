/*
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ADVISER_H
#define ADVISER_H

#include "abstract_piece.h"

class adviser : public abstract_piece
{
public:
    explicit adviser(std::int8_t _file = 0, std::int8_t _rank = 0, bool oppo = false) : abstract_piece(_file, _rank, oppo) {}
    explicit adviser(const adviser &b) : abstract_piece(b) {}

    abstract_piece* make_a_copy() const;

    char abbr_name() const;
    std::string print_name(const bool &chinese = false) const;
    int value() const;
    
protected:
    void gen_moves(const board &m_board);

private:
    static const position upleft;
    static const position upright;
    static const position downleft;
    static const position downright;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(abstract_piece);
    }
};

#endif //ADVISER_H
