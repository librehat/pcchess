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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>

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

    abstract_player *red, *black;

    cout << "#================================================================================" << endl;
    cout << "#  Generated Time: " << boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time()) << endl;
    cout << "#  Red   player  : " << (human ? "human_player" : "uct_player") << endl;
    cout << "#  Black player  : " << "threaded_uct_player" << endl;
    cout << "#  Total games   : " << games << endl;
    cout << "#================================================================================" << endl;
    cout << "# Sequence  Rounds  Red Score  Black Score   Red Simulations    Black Simulations" << endl;

    for (int i = 0; i < games; ++i) {
        if (human) {
            red = new human_player(chinese_print, true);
        } else {
            red = new uct_player(true);
        }
        black = new threaded_uct_player(0, false);
        red->init_pieces();
        black->init_pieces();
        game g(red, black);
        abstract_player* winner = g.playout();

        if (enable_print) {
            g.print_board(chinese_print);
        }

        int red_score = 0, black_score = 0;
        if (winner == red) {
            red_score = 1;
            black_score = -1;
        } else if (winner == black) {
            red_score = -1;
            black_score = 1;
        }

        cout << boost::format("  %-9d %-7d %=9d  %=11d   %-17u  %-17u\n") % i % g.get_rounds() % red_score % black_score % red->get_total_simulations() % black->get_total_simulations();

        delete red;
        delete black;
    }

    return 0;
}
