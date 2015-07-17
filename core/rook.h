/*
 * Rook, chariot.
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ROOK_H
#define ROOK_H

#include "abstract_piece.h"

class rook : public abstract_piece
{
public:
    rook(int _file = 0, int _rank = 0, bool oppo = false) : abstract_piece(_file, _rank, oppo) {}
    rook(const rook &b) : abstract_piece(b) {}

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

#endif //ROOK_H
