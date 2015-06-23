/*
 * let a computer play with a computer
 * output the win, time, etc. in the end
 */
#include "../core/game.h"
#include "../core/random_player.h"

int main(int argc, char** argv)
{
    game g;
    auto our = new random_player(g.get_board());
    auto opp = new random_player(g.get_board());
    our->init_pieces(false);
    opp->init_pieces(true);
    g.setup_players(our, opp);

    position from, to;
    bool movable = false;

    while (true) {
        movable = our->think_next_move(&from, &to);
        if (!movable || g.is_over()) {
            break;
        } else {
            g.move_piece(from, to);
        }
        movable = opp->think_next_move(&from, &to);
        if (!movable || g.is_over()) {
            break;
        } else {
            g.move_piece(from, to);
        }
    }

    g.print_board();

    return 0;
}
