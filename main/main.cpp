/*
 * let a computer play with a computer
 * output the win, time, etc. in the end
 */

#include "../core/game.h"
#include "../core/random_player.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    board bd;
    random_player our(bd);
    random_player opp(bd);
    our.init_pieces(false);
    opp.init_pieces(true);
    game g(&our, &opp, bd);
    abstract_player* winner = g.playout();

    g.print_board(true);

    if (winner == &our) {
        cout << "We won" << endl;
    } else if (winner == &opp) {
        cout << "Opp won" << endl;
    } else {
        cout << "Draw" << endl;
    }

    return 0;
}
