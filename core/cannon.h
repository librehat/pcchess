/*
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef CANNON_H
#define CANNON_H

#include "abstract_piece.h"

class cannon : public abstract_piece
{
public:
    cannon(int _file, int _rank, bool oppo) : abstract_piece(_file, _rank, oppo) {}
    cannon(const cannon &b) : abstract_piece(b) {}

    abstract_piece* make_a_copy() const;

    char abbr_name() const;
    std::string chinese_name() const;
    int value() const;
    
protected:
    void gen_moves(const board &m_board);

private:
    void search_moves(const bool forward, const bool is_rank, const board &m_board);

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<abstract_piece>(*this);
    }
};

#endif //CANNON_H
