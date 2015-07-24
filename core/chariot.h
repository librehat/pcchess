/*
 * ChaRiot, also known as chariot
 *
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef CHARIOT_H
#define CHARIOT_H

#include "abstract_piece.h"

class chariot : public abstract_piece
{
public:
    explicit chariot(int _file = 0, int _rank = 0, bool oppo = false) : abstract_piece(_file, _rank, oppo) {}
    explicit chariot(const chariot &b) : abstract_piece(b) {}

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
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(abstract_piece);
    }
};

#endif //CHARIOT_H
