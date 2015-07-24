/*
 * let a computer play with a computer
 * output the win, time, etc. in the end
 */

#include "core/game.h"
#include "player/random_player.h"
#include "player/uct_player.h"
#include "player/threaded_uct_player.h"
#include "unistd.h"
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char** argv)
{
    int opt, rounds = 1;
    bool enable_print = false, chinese_print = false;

    while((opt = getopt(argc, argv, "r:t:pch")) != -1) {
        switch(opt) {
        case 'r':
            rounds = atoi(optarg);
            break;
        case 't':
            game::step_time = atol(optarg);
            break;
        case 'p':
            enable_print = true;
            break;
        case 'c':
            chinese_print = true;
            break;
        default:
            cout << "Command-line options:\n"
                 << "  -r <number of rounds>\n"
                 << "  -t <maximum think time (milliseconds)>\n"
                 << "  -p\tprint out the board after each round\n"
                 << "  -c\tuse Chinese characters in the board\n"
                 << endl;
            return 1;
        }
    }

    int we_win = 0;
    int we_draw = 0;
    int we_lose = 0;
    int our_sims = 0;
    int opp_sims = 0;
    for (int i = 0; i < rounds; ++i) {
        uct_player *red, *black;
        red = new threaded_uct_player(false);
        black = new uct_player(true);
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

        ofstream fs;
        fs.open("/tmp/xml_archive.xml", ios_base::out);
        uct_player::xml_archive_tree(fs, red->get_tree());
        fs.close();

        delete red;
        delete black;
    }

    cout << "WIN:\t" << we_win << endl
         << "DRAW:\t" << we_draw << endl
         << "LOSE:\t" << we_lose << endl
         << "TOTAL SIMULATIONS:\t" << our_sims << " v.s. " << opp_sims << endl;

    return 0;
}
