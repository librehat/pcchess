#ifndef ABSTRACT_PlAYER_H
#define ABSTRACT_PlAYER_H

#include "abstract_piece.h"
#include "position.h"
#include "pos_move.h"
#include <list>
#include <tuple>
#include <boost/serialization/list.hpp>

class abstract_player
{
public:
    /*
     * red vs black
     * the black side is in the north
     */
    explicit abstract_player(bool red = true);
    explicit abstract_player(const abstract_player &b);
    virtual ~abstract_player();

    void add(p_piece p);
    void remove(p_piece p);//if p doesn't belong to this player, it'll throw an error

    /*
     * initialise the pieces list to a normal beginning situation
     * no need to call add function multiple times
     */
    void init_pieces();

    bool is_redside() const;
    bool is_checked() const;
    bool is_checkmated() const;
    const std::list<p_piece> &get_pieces() const;

    /*
     * sub-class has to implement this pure virtual function
     * return true if a move is proposed and store the results
     * in arguments.
     * otherwise, return false if no move can be made
     */
    virtual bool think_next_move(pos_move &m, const board &bd, const std::string &fen, unsigned int no_eat_half_rounds, const std::vector<pos_move> &banmoves) = 0;

    /*
     * the sub-class player might want to change some strategies
     * after the opponent moved.
     * this function does nothing by default
     */
    virtual void opponent_moved(const pos_move &) {}

    virtual std::int64_t get_total_simulations() const { return 0; }

protected:
    std::list<p_piece> pieces;
    const bool red_side;
    bool checked;//if our general is checked
    bool checkmated;//if our general is dead

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_NVP(pieces);
        ar & boost::serialization::make_nvp("red_side", const_cast<bool &>(red_side));
        ar & BOOST_SERIALIZATION_NVP(checked);
        ar & BOOST_SERIALIZATION_NVP(checkmated);
    }
};

BOOST_SERIALIZATION_ASSUME_ABSTRACT(abstract_player)

#endif //ABSTRACT_PlAYER_H
