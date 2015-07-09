/*
 * let a computer play with a computer
 * output the win, time, etc. in the end
 */

#include "../core/game.h"
#include "../core/random_player.h"
#include "../core/uct_player.h"
#include "../core/threaded_uct_player.h"
#include "unistd.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    int opt, rounds = 1, think_time = 2;
    bool enable_print = false, chinese_print = false, all_random = false;

    while((opt = getopt(argc, argv, "r:t:pca")) != -1) {
        switch(opt) {
        case 'r':
            rounds = atoi(optarg);
            break;
        case 't':
            think_time = atoi(optarg);
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
            cout << "Command-line options:\n"
                 << "  -r <number of rounds>\n"
                 << "  -t <maximum think time (s)>\n"
                 << "  -p\tprint out the board after each round\n"
                 << "  -c\tuse Chinese characters in the board\n"
                 << "  -a\tuse two random players instead of one UCT and one random"
                 << endl;
            return 1;
        }
    }

    int we_win = 0;
    int we_draw = 0;
    int we_lose = 0;
    for (int i = 0; i < rounds; ++i) {
        abstract_player *our;
        uct_player *opp;
        //opp = new random_player;
        opp = new uct_player(think_time, false, nullptr);
        if (all_random) {
            our = new random_player;
        } else {
            //our = new uct_player(think_time, true, opp);
            our = new threaded_uct_player(think_time, true, opp, 4);
        }
        opp->set_opponent_player(our);
        our->init_pieces(false);
        opp->init_pieces(true);
        game g(our, opp);
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
