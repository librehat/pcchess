/*
 * let a computer play with a computer
 * output the win, time, etc. in the end
 */

#include "../core/game.h"
#include "../core/random_player.h"
#include "../core/mcts_player.h"
#include "unistd.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    int opt, rounds = 1;
    bool enable_print = false, chinese_print = false, all_random = false;

    while((opt = getopt(argc, argv, "r:pca")) != -1) {
        switch(opt) {
        case 'r':
            rounds = atoi(optarg);
            break;
        case 'p':
            enable_print = true;
            break;
        case 'c':
            chinese_print = true;
            break;
        case 'a':
            all_random = true;
            break;
        default:
            ;//TODO
            return 1;
        }
    }

    int we_win = 0;
    int we_draw = 0;
    int we_lose = 0;
    for (int i = 0; i < rounds; ++i) {
        board bd;
        abstract_player *our, *opp;
        opp = new random_player;
        if (all_random) {
            our = new random_player;
        } else {
            our = new mcts_player(2, true, opp);
        }
        our->init_pieces(bd, false);
        opp->init_pieces(bd, true);
        game g(our, opp, bd);
        abstract_player* winner = g.playout();

        if (enable_print) {
            g.print_board(chinese_print);
        }

        if (winner == our) {
            we_win++;
        } else if (winner == opp) {
            we_lose++;
        } else {
            we_draw++;
        }

        delete our;
        delete opp;
    }

    cout << "WIN:\t" << we_win << endl
         << "DRAW:\t" << we_draw << endl
         << "LOSE:\t" << we_lose << endl;

    return 0;
}
