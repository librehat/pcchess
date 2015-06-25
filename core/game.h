#ifndef GAME_H
#define GAME_H

#include "abstract_piece.h"
#include "abstract_player.h"
#include "board.h"

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
    board& get_board_ref();
    void print_board(bool chinese_char = false) const;//print current chess board into stdout
    bool is_over() const;

private:
    board m_board;
    abstract_player *our_player;
    abstract_player *opp_player;

    bool game_over;
};

#endif //GAME_H
