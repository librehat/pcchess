#include "core/game.h"
#include "player/random_player.h"
#include "player/root_uct_player.h"
#include "player/slow_tree_uct_player.h"
#include "player/uct_treesplit_player.h"
#include <boost/mpi.hpp>
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

    while (games-- > 0) {
        root_uct_player *red;
        abstract_player *black = new uct_player(false);

        switch (player_id) {
        case 1:
            red = new root_uct_player(true);
            break;
        case 2:
            red = new slow_tree_uct_player(300, true);
            break;
        case 3:
            red = new uct_treesplit_player(0, true);
            break;
        default:
            cerr << "bad player_id" << endl;
            return 2;
        }

        red->init_pieces();
        black->init_pieces();

        world_comm.barrier();
        if (world_comm.rank() == 0) {//master plays the game
            game g(red, black);
            abstract_player* winner = g.playout();
            if (winner == red) {
                cout << "Won" << endl;
            } else if (winner == black) {
                cout << "Lost" << endl;
            } else {
                cout << "Draw" << endl;
            }

            if (enable_print) {
                g.print_board(chinese_print);
            }

            cout << "Total rounds in this game: " << g.get_rounds() << endl;
            cout << "Total simulations: " << red->get_total_simulations() << " vs " << black->get_total_simulations() << endl;
        } else {
            red->do_slave_job();
        }
        delete red;
        delete black;
        node::reset_simulation_count();
    }

    return 0;
}
