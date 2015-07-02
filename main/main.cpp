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
    int we_win = 0;
    int we_draw = 0;
    int we_lose = 0;
    for (int i = 0; i < 200; ++i) {
        board bd;
        random_player our, opp;
        our.init_pieces(bd, false);
        opp.init_pieces(bd, true);
        game g(&our, &opp, bd);
        abstract_player* winner = g.playout();

        if (winner == &our) {
            we_win++;
        } else if (winner == &opp) {
            we_lose++;
        } else {
            we_draw++;
        }
    }

    cout << "WIN:\t" << we_win << endl
         << "DRAW:\t" << we_draw << endl
         << "LOSE:\t" << we_lose << endl;

    return 0;
}
