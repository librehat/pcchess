#ifndef RANDOM_PLAYER_H
#define RANDOM_PLAYER_H

#include "abstract_player.h"
#include <random>

class random_player : public abstract_player
{
public:
    random_player(abstract_piece*** _board);

    bool think_next_move(position *from, position *to);

private:
    std::default_random_engine generator;
};

#endif //RANDOM_PLAYER_H
