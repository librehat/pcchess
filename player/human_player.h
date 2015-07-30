#ifndef HUMAN_PLAYER_H
#define HUMAN_PLAYER_H

#include "../core/abstract_player.h"

class human_player : public abstract_player
{
public:
    explicit human_player(bool chinese_char = false, bool red = true) : abstract_player(red), chinese_character(chinese_char) {}

    bool think_next_move(pos_move &m, const board &bd, std::uint8_t no_eat_half_rounds, const std::vector<pos_move> &banmoves);
    void opponent_moved(const pos_move &);

private:
    bool chinese_character;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(abstract_player);
        ar & BOOST_SERIALIZATION_NVP(chinese_character);
    }
};

#endif //HUMAN_PLAYER_H
