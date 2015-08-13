/*
 * let a computer play with a computer
 * output the win, time, etc. in the end
 */

#include "core/game.h"
#include "player/random_player.h"
#include "player/uct_player.h"
#include "player/threaded_uct_player.h"
#include "player/human_player.h"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char** argv)
{
    long step_time;
    int games;
    uint8_t no_eat_rounds;

    po::options_description desc("Options");
    desc.add_options()
            ("help,h", "display this help and exit")
            ("games,g", po::value<int>(&games)->default_value(1), "number of games to play")
            ("step-time", po::value<long>(&step_time)->default_value(1000), "maximum think time (milliseconds)")
            ("max-no-eat", po::value<uint8_t>(&no_eat_rounds)->default_value(60), "maximum rounds when no piece gets eaten, set to 0 to disable this feature")
            ("print,p", "print out the board after each game")
            ("disable-header", "don't print out the header")
            ("chinese,c", "use Chinese characters in the board")
            ("human,H", "let human player join this game")
            ("black-first", "let black player make the first move");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << endl;
        return 1;
    }

    game::step_time = step_time;
    game::NO_EAT_DRAW_HALF_ROUNDS = 2 * no_eat_rounds;

    bool enable_print = vm.count("print"), chinese_print = vm.count("chinese"), human = vm.count("human"), disable_header = vm.count("disable-header"), red_first = !vm.count("black-first");

    abstract_player *red, *black;

    if (!disable_header) {
        cout << "#================================================================================" << endl;
        cout << "#  Generated Time: " << boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time()) << endl;
        cout << "#  Red   player  : " << (human ? "human_player" : "uct_player") << endl;
        cout << "#  Black player  : " << "threaded_uct_player" << endl;
        cout << "#  Total games   : " << games << endl;
        cout << "#================================================================================" << endl;
        cout << "# Sequence  Rounds  Red Score  Black Score   Red Simulations    Black Simulations" << endl;
    }

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
        abstract_player* winner = g.playout(red_first);

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
