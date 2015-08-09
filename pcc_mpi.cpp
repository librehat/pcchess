#include "core/game.h"
#include "player/random_player.h"
#include "player/root_uct_player.h"
#include "player/slow_tree_uct_player.h"
#include "player/uct_treesplit_player.h"
#include "player/treesplit_node.h"
#include <cstdint>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>

using namespace std;
namespace mpi = boost::mpi;
namespace po  = boost::program_options;

int main(int argc, char **argv)
{
    mpi::environment env(argc, argv, mpi::threading::funneled);
    mpi::communicator world_comm;
    std::ignore = env;

    int games, player_id;//1: root_uct_player, 2: slow_tree_uct_player, 3: uct_treesplit_player

    po::options_description desc("Options");
    desc.add_options()
            ("help,h", "display this help and exit")
            ("games,g", po::value<int>(&games)->default_value(1), "number of games to play")
            ("step_time", po::value<long>(), "maximum think time (milliseconds)")
            ("max_no_eat", po::value<uint8_t>(), "maximum rounds when no piece gets eaten, set to 0 to disable this feature")
            ("print,p", "print out the board after each game")
            ("chinese,c", "use Chinese characters in the board")
            ("player_id,s", po::value<int>(&player_id)->default_value(1), "set red player. 1: root, 2: slow-tree, 3: treesplit");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
    po::notify(vm);

    if (vm.count("help") && world_comm.rank() == 0) {
        cout << desc << endl;
        return 1;
    }

    if (vm.count("step_time")) {
        game::step_time = vm["step_time"].as<long>();
    }
    if (vm.count("max_no_eat")) {
        game::NO_EAT_DRAW_HALF_ROUNDS = 2 * vm["max_no_eat"].as<uint8_t>();
    }

    bool enable_print = vm.count("print"), chinese_print = vm.count("chinese");

    if (world_comm.rank() == 0) {
        cout << "#================================================================================" << endl;
        cout << "#  Generated Time: " << boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time()) << endl;
        cout << "#  MPI World Size: " << world_comm.size() << endl;
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
            treesplit_node::clear_transtable();
#ifdef _DEBUG
            cout << "[" << world_comm.rank() << "] transtable size: " << treesplit_node::transtable_size() << endl;
#endif
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
