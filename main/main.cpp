/*
 * let a computer play with a computer
 * output the win, time, etc. in the end
 */
#include "../core/game.h"
#include "../core/random_player.h"

int main(int argc, char** argv)
{
    game g;
    abstract_player* our = new random_player(g.get_board_ref());
    abstract_player* opp = new random_player(g.get_board_ref());
    our->init_pieces(false);
    opp->init_pieces(true);
    g.setup_players(our, opp);

    pos_move next_move;
    bool movable = false;

    while (true) {
        movable = our->think_next_move(next_move);
        if (!movable || g.is_over()) {
            break;
        } else {
            g.move_piece(next_move);
        }
        movable = opp->think_next_move(next_move);
        if (!movable || g.is_over()) {
            break;
        } else {
            g.move_piece(next_move);
        }
    }

    g.print_board(true);

    return 0;
}
