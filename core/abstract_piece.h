/*
 * Abstract piece class
 *
 * This is the base class for all pieces
 * 
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ABSTRACT_PIECE_H
#define ABSTRACT_PIECE_H

#include "position.h"
#include "board.h"
#include <vector>
#include <string>
#include <boost/serialization/vector.hpp>

class abstract_piece
{
public:
    explicit abstract_piece(std::int8_t _file = 0, std::int8_t _rank = 0, bool red = true);
    explicit abstract_piece(const abstract_piece &b);
    virtual ~abstract_piece();

    /*
     * we need to take care of the coordinate system for different sides
     */
    bool is_redside() const;

    const position& get_position() const;

    void move_to_pos(int newfile, int newrank);
    void move_to_pos(const position &new_pos);
    
    /*
     * re-generate avail_moves
     * give the board data to this function
     */
    void update_moves(const board &m_board);
    const std::vector<position>& get_avail_target_positions() const;
    bool is_movable() const;//check if avail_moves is empty

    virtual char abbr_name() const = 0;
    virtual std::string print_name(const bool &chinese = false) const = 0;
    virtual int value() const = 0;//the "value" of this piece indicates how important it is
    virtual bool is_king() const { return false; }

    virtual abstract_piece* make_a_copy() const = 0;

    bool operator ==(const abstract_piece &b);
    bool operator !=(const abstract_piece &b);

protected:
    const bool red_side;
    position pos;
    std::vector<position> avail_pos;

    /*
     * a piece can't move if it's the only piece separate generals
     * return false if it's such a piece hence it can't move
     * return true if it can move freely
     */
    bool can_i_move(const board &m_board) const;
    
    virtual void gen_moves(const board &m_board) = 0;
    //invalid: out of the given scope, default to the board
    void remove_invalid_moves(const board &m_board, std::int8_t min_file = 0, std::int8_t max_file = 8, std::int8_t min_rank = 0, std::int8_t max_rank = 9);

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & boost::serialization::make_nvp("red_side", const_cast<bool &>(red_side));
        ar & BOOST_SERIALIZATION_NVP(pos);
        ar & BOOST_SERIALIZATION_NVP(avail_pos);
    }
};

BOOST_SERIALIZATION_ASSUME_ABSTRACT(abstract_piece)

#endif //ABSTRACT_PIECE_H
