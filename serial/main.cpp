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
        uct_player *opp, *our;
        opp = new uct_player(true);
        our = new threaded_uct_player(false);
        our->init_pieces();
        opp->init_pieces();
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

        our_sims += our->get_total_simulations();
        opp_sims += opp->get_total_simulations();

        ofstream fs;
        fs.open("/tmp/xml_archive.xml", ios_base::out);
        uct_player::xml_archive_tree(fs, our->get_tree());
        fs.close();

        delete our;
        delete opp;
    }

    cout << "WIN:\t" << we_win << endl
         << "DRAW:\t" << we_draw << endl
         << "LOSE:\t" << we_lose << endl
         << "TOTAL SIMULATIONS:\t" << our_sims << " v.s. " << opp_sims << endl;

    return 0;
}
