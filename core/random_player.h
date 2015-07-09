#ifndef RANDOM_PLAYER_H
#define RANDOM_PLAYER_H

#include "abstract_player.h"
#include <random>

class random_player : public abstract_player
{
public:
    random_player(bool opposite = false);
    random_player(const abstract_player &b);

    bool think_next_move(pos_move &_move, const board &bd);

private:
    static std::random_device device;
    std::default_random_engine generator;
};

#endif //RANDOM_PLAYER_H
