#include "core/game.h"
#include "player/random_player.h"
#include "player/root_uct_player.h"
#include "player/slow_tree_uct_player.h"
#include "player/uct_treesplit_player.h"
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <unistd.h>

using namespace std;
namespace mpi = boost::mpi;

int main(int argc, char **argv)
{
    mpi::environment env(argc, argv, mpi::threading::funneled);
    mpi::communicator world_comm;
    std::ignore = env;

    int opt, games = 1, player_id = 1;//1: root_uct_player, 2: slow_tree_uct_player, 3: uct_treesplit_player
    bool enable_print = false, chinese_print = false;

    while((opt = getopt(argc, argv, "g:t:n:s:pc")) != -1) {
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
        case 's':
            player_id = atoi(optarg);
            break;
        case 'p':
            enable_print = true;
            break;
        case 'c':
            chinese_print = true;
            break;
        default:
            if (world_comm.rank() == 0) {
                cout << "Command-line options:\n"
                     << "  -g <number of games>\n"
                     << "  -t <maximum think time (milliseconds)>\n"
                     << "  -n <maximum rounds when no piece gets eaten>\tset to 0 to disable this feature\n"
                     << "  -s <player id>\n"
                     << "     1: root_uct_player\n"
                     << "     2: slow_tree_uct_player\n"
                     << "     3: uct_treesplit_player\n"
                     << endl;
            }
            return 1;
        }
    }

    if (world_comm.rank() == 0) {
        cout << "#================================================================================" << endl;
        cout << "#  Generated Time: " << boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time()) << endl;
        cout << "#  Red   player  : " << (player_id == 1 ? "root_uct_player" : player_id == 2 ? "slow_tree_uct_player" : "uct_treesplit_player") << endl;
        cout << "#  Black player  : " << "uct_player" << endl;
        cout << "#  Total games   : " << games << endl;
        cout << "#================================================================================" << endl;
        cout << "# Sequence  Rounds  Red Score  Black Score   Red Simulations    Black Simulations" << endl;
    }

    for (int i = 0; i < games; ++i) {
        root_uct_player *red;
        switch (player_id) {
        case 1:
            red = new root_uct_player(true);
            break;
        case 2:
            red = new slow_tree_uct_player(500, true);
            break;
        case 3:
            red = new uct_treesplit_player(0, true);
            break;
        default:
            cerr << "bad player_id" << endl;
            return 2;
        }
        red->init_pieces();

        abstract_player *black = new uct_player(false);
        black->init_pieces();

        world_comm.barrier();
        if (world_comm.rank() == 0) {//master plays the game
            game g(red, black);
            abstract_player* winner = g.playout();

            int red_score = 0, black_score = 0;
            if (winner == red) {
                red_score = 1;
                black_score = -1;
            } else if (winner == black) {
                red_score = -1;
                black_score = 1;
            }

            if (enable_print) {
                g.print_board(chinese_print);
            }

            cout << boost::format("  %-9d %-7d %=9d  %=11d   %-17u  %-17u\n") % i % g.get_rounds() % red_score % black_score % red->get_total_simulations() % black->get_total_simulations();
        } else {
            red->do_slave_job();
        }
        delete red;
        delete black;
    }

    return 0;
}
