#ifndef GAME_H
#define GAME_H

#include "abstract_piece.h"
#include <list>

class game
{
public:
    game();
    ~game();

    void move_piece(const position &from, const position &to);

private:
    abstract_piece*** board;
    abstract_piece**  board_data;

    std::list<abstract_piece*> our_alive_pieces;
    std::list<abstract_piece*> opp_alive_pieces;
};

#endif //GAME_H
