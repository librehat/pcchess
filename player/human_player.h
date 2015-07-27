#ifndef HUMAN_PLAYER_H
#define HUMAN_PLAYER_H

#include "../core/abstract_player.h"

class human_player : public abstract_player
{
public:
    explicit human_player(bool red = true) : abstract_player(red) {}

    bool think_next_move(pos_move &m, const board &bd, std::int8_t no_eat_half_rounds, const std::vector<pos_move> &banmoves);
    void opponent_moved(const pos_move &);
};

#endif //HUMAN_PLAYER_H
