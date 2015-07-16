/*
 * General, also known as King in some translations.
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef GENERAL_H
#define GENERAL_H

#include "abstract_piece.h"

class general : public abstract_piece
{
public:
    general(int _file, int _rank, bool oppo) : abstract_piece(_file, _rank, oppo) {}
    general(const general &b) : abstract_piece(b) {}

    abstract_piece* make_a_copy() const;

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves(const board &m_board);

private:
    static const position up;
    static const position down;
    static const position left;
    static const position right;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<abstract_piece>(*this);
    }
};

#endif //GENERAL_H
