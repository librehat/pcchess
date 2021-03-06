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
    explicit chariot(std::int8_t _file = 0, std::int8_t _rank = 0, bool oppo = false) : abstract_piece(_file, _rank, oppo) {}
    explicit chariot(const chariot &b) : abstract_piece(b) {}

    abstract_piece* make_a_copy() const;

    char abbr_name() const;
    std::string print_name(const bool &chinese = false) const;
    int value() const;
    
protected:
    void gen_moves(const board &m_board);

private:
    void search_moves(const bool forward, const bool is_rank, const board &m_board);

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(abstract_piece);
    }
};

#endif //CHARIOT_H
