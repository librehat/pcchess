#ifndef ABSTRACT_PlAYER_H
#define ABSTRACT_PlAYER_H

#include "abstract_piece.h"
#include "position.h"
#include <list>
#include <boost/serialization/list.hpp>

class abstract_player
{
public:
    /* opposite: whether this player is opposite player */
    abstract_player(bool opposite = false);
    abstract_player(const abstract_player &b);
    virtual ~abstract_player();

    void add(p_piece p);
    void remove(p_piece p);//if p doesn't belong to this player, it'll throw an error
    void add_history(const position &from, const position &to);
    void add_history(const pos_move &m);
    const std::list<pos_move>& get_history() const;
    void clear_history();

    /*
     * initialise the pieces list to a normal beginning situation
     * no need to call add function multiple times
     */
    void init_pieces();

    bool is_opposite() const;
    bool is_checked() const;
    bool is_checkmated() const;
    const std::list<p_piece> &get_pieces() const;

    /*
     * sub-class has to implement this pure virtual function
     * return true if a move is proposed and store the results
     * in arguments.
     * otherwise, return false if no move can be made
     */
    virtual bool think_next_move(pos_move &m, const board &bd, const abstract_player &opponent) = 0;

    /*
     * the sub-class player might want to change some strategies
     * after the opponent moved.
     * this function does nothing by default
     */
    virtual void opponent_moved(const pos_move &, const abstract_player &opponent) {}

    virtual int get_total_simulations() const { return 0; }

protected:
    std::list<p_piece> pieces;
    std::list<pos_move> move_history;//front: latest move, back: the first move
    const bool opposite_player;
    bool checked;//if our general is checked
    bool checkmated;//if our general is dead

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(pieces);
        ar & BOOST_SERIALIZATION_NVP(move_history);
        ar & boost::serialization::make_nvp("opposite_player", const_cast<bool &>(opposite_player));
        ar & BOOST_SERIALIZATION_NVP(checked);
        ar & BOOST_SERIALIZATION_NVP(checkmated);
    }
};

BOOST_SERIALIZATION_ASSUME_ABSTRACT(abstract_player)

#endif //ABSTRACT_PlAYER_H
