/*
 * random_player plays the game totally randomly based on currently available moves
 */
#ifndef RANDOM_PLAYER_H
#define RANDOM_PLAYER_H

#include "../core/abstract_player.h"

class random_player : public abstract_player
{
public:
    explicit random_player(bool red = true);
    explicit random_player(const abstract_player &b);

    bool think_next_move(pos_move &_move, const board &bd, unsigned int no_eat_rounds, const std::vector<pos_move> &banmoves);

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(abstract_player);
    }
};

#endif //RANDOM_PLAYER_H
