#ifndef GAME_H
#define GAME_H

#include "position.h"
#include "abstract_piece.h"
#include "abstract_player.h"
#include "board.h"

class game
{
public:
    game(abstract_player* our, abstract_player* opp);
    ~game();

    void print_board(bool chinese_char = false) const;//print current chess board into stdout

    /*
     * play the game till the end and return the winner
     * if the game result is draw, then a nullptr will be returned
     */
    abstract_player* playout(bool we_first = true);
    bool play_single_move(const pos_move &_move, bool we);//true if both players moved

private:
    void setup_players();
    void move_piece(const position &from, const position &to);
    void move_piece(const pos_move &_move);

    board m_board;
    abstract_player *our_player;
    abstract_player *opp_player;
};

#endif //GAME_H
