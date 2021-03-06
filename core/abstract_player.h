#ifndef ABSTRACT_PlAYER_H
#define ABSTRACT_PlAYER_H

#include "abstract_piece.h"
#include "position.h"
#include "pos_move.h"

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
     * initialise the pieces vector to a normal beginning situation
     * no need to call add function multiple times
     */
    void init_pieces();

    inline bool is_redside() const { return red_side; }
    inline bool is_in_check() const { return in_check; }
    inline bool is_checkmated() const { return checkmated; }
    const std::vector<p_piece> &get_pieces() const { return pieces; }
    p_piece get_king();//return the pointer to king, nullptr is returned if no king present
    position get_king_position();

    inline void set_check(const bool &c) { in_check = c; }

    /*
     * sub-class has to implement this pure virtual function
     * return true if a move is proposed and store the results
     * in arguments.
     * otherwise, return false if no move can be made
     */
    virtual bool think_next_move(pos_move &m, const board &bd, std::uint8_t no_eat_half_rounds, const std::vector<pos_move> &banmoves) = 0;

    /*
     * the sub-class player might want to change some strategies
     * after the opponent moved.
     * this function does nothing by default
     */
    virtual void opponent_moved(const pos_move &) {}

    virtual std::uint64_t get_total_simulations() const { return 0; }
    std::vector<position> get_all_available_target_positions(const board &bd) const;
    std::vector<pos_move> get_all_available_moves(const board &bd) const;

protected:
    std::vector<p_piece> pieces;
    const bool red_side;
    bool in_check;//if our general is in check
    bool checkmated;//if our general is dead

    p_piece pking;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_NVP(pieces);
        ar & boost::serialization::make_nvp("red_side", const_cast<bool &>(red_side));
        ar & BOOST_SERIALIZATION_NVP(in_check);
        ar & BOOST_SERIALIZATION_NVP(checkmated);
        ar & BOOST_SERIALIZATION_NVP(pking);
    }
};

BOOST_SERIALIZATION_ASSUME_ABSTRACT(abstract_player)

#endif //ABSTRACT_PlAYER_H
