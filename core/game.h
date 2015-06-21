#ifndef GAME_H
#define GAME_H

#include "abstract_piece.h"
#include "abstract_player.h"

class game
{
public:
    game();
    ~game();

    void move_piece(const position &from, const position &to);

private:
    abstract_piece*** board;
    abstract_piece**  board_data;

    abstract_player *our_player;
    abstract_player *opp_player;
};

#endif //GAME_H
