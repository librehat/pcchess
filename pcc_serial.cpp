/*
 * let a computer play with a computer
 * output the win, time, etc. in the end
 */

#include "core/game.h"
#include "player/random_player.h"
#include "player/uct_player.h"
#include "player/threaded_uct_player.h"
#include "player/human_player.h"
#include <unistd.h>
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char** argv)
{
    int opt, games = 1;
    bool enable_print = false, chinese_print = false, human = false;

    while((opt = getopt(argc, argv, "g:t:n:pch")) != -1) {
        switch(opt) {
        case 'g':
            games = atoi(optarg);
            break;
        case 't':
            game::step_time = atol(optarg);
            break;
        case 'n':
            game::NO_EAT_DRAW_HALF_ROUNDS = 2 * static_cast<uint8_t>(atoi(optarg));
            break;
        case 'p':
            enable_print = true;
            break;
        case 'c':
            chinese_print = true;
            break;
        case 'h':
            human = true;
            break;
        default:
            cout << "Command-line options:\n"
                 << "  -g <number of games>\n"
                 << "  -t <maximum think time (milliseconds)>\n"
                 << "  -n <maximum rounds when no piece gets eaten>\tset to 0 to disable this feature\n"
                 << "  -p\tprint out the board after each round\n"
                 << "  -c\tuse Chinese characters in the board\n"
                 << "  -h\tlet human player join in this game\n"
                 << endl;
            return 1;
        }
    }

    int we_win = 0;
    int we_draw = 0;
    int we_lose = 0;
    int64_t our_sims = 0;
    int64_t opp_sims = 0;
    abstract_player *red, *black;
    for (int i = 0; i < games; ++i) {
        if (human) {
            red = new human_player(chinese_print, true);
        } else {
            red = new random_player(true);
        }
        black = new threaded_uct_player(0, false);
        //black = new uct_player(false);
        red->init_pieces();
        black->init_pieces();
        game g(red, black);
        abstract_player* winner = g.playout();

        if (enable_print) {
            g.print_board(chinese_print);
        }

        if (winner == red) {
            we_win++;
        } else if (winner == black) {
            we_lose++;
        } else {
            we_draw++;
        }

        our_sims += red->get_total_simulations();
        opp_sims += black->get_total_simulations();
        delete red;
        delete black;
    }

    cout << "WIN:\t" << we_win << endl
         << "DRAW:\t" << we_draw << endl
         << "LOSE:\t" << we_lose << endl
         << "TOTAL SIMULATIONS:\t" << our_sims << " v.s. " << opp_sims << endl;

    return 0;
}
