#ifndef RANDOM_PLAYER_H
#define RANDOM_PLAYER_H

#include "abstract_player.h"

class random_player : public abstract_player
{
public:
    random_player(const abstract_player* const opp = nullptr, bool opposite = false);
    random_player(const abstract_player &b);

    bool think_next_move(pos_move &_move, const board &bd);

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(abstract_player);
    }
};

#endif //RANDOM_PLAYER_H
