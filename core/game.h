#ifndef GAME_H
#define GAME_H

#include "abstract_piece.h"
#include "abstract_player.h"

class game
{
public:
    game();
    ~game();

    /*
     * this function should be called and only be called once to setup players
     * players have to be valid during the game (using new to create such a player)
     * this game instance will then take control of players' memory
     * therefore don't delete them explicitly outside game
     */
    void setup_players(abstract_player* our, abstract_player *opp);

    void move_piece(const position &from, const position &to);
    abstract_piece*** get_board() const;
    void print_board() const;//print current chess board into stdout
    bool is_over() const;

private:
    abstract_piece*** board;
    abstract_piece**  board_data;

    abstract_player *our_player;
    abstract_player *opp_player;

    bool game_over;
};

#endif //GAME_H
