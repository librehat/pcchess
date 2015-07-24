#ifndef GAME_H
#define GAME_H

#include "position.h"
#include "abstract_piece.h"
#include "abstract_player.h"
#include "board.h"
#include <deque>

class game
{
public:
    explicit game(abstract_player* _red, abstract_player* _black, unsigned int not_eat_rounds = 0);
    ~game();

    void print_board(bool chinese_char = false) const;//print current chess board into stdout

    /*
     * play the game till the end and return the winner
     * if the game result is draw, then a nullptr will be returned
     */
    abstract_player *playout(bool red_first = true);

    void move_piece(const position &from, const position &to);
    void move_piece(const pos_move &_move);

    /*
     * Parse position FEN string according to UCCI protocol
     * The players' piece list should be empty before calling this function,
     * because this function would add pieces to players according to the
     * FEN string. For details, check
     * http://www.xqbase.com/protocol/cchess_fen.htm
     */
    void parse_fen(const std::string &fen);

    std::deque<pos_move> get_history() const;

    static long int step_time;//maximum time for each step (milliseconds)

    /*
     * rounds can be played after last time a piece got eaten
     * after this limit, the game result is considered as draw
     * default: 60 (this could be changed depending on the specific game)
     * set to 0 to disable this rule (WARN: could result in an endless loop)
     */
    static unsigned int NO_EAT_DRAW_ROUNDS;

private:
    void setup_players();

    board m_board;
    abstract_player *const red;
    abstract_player *const black;

    std::deque<pos_move> history;
    unsigned int rounds_since_last_eat;
};

#endif //GAME_H
