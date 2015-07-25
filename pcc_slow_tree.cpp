#include "core/game.h"
#include "player/random_player.h"
#include "player/uct_player.h"
#include "player/threaded_uct_player.h"
#include "player/slow_tree_uct_player.h"
#include "player/serialization_export.h"
#include <boost/mpi.hpp>

using namespace std;
namespace mpi = boost::mpi;

int main(int argc, char **argv)
{
    /*
     * the constructor of environment is equivalent to MPI_Init
     * and the destructor is to MPI_Finalize
     */
    mpi::environment env(argc, argv);
    mpi::communicator world_comm;
    std::ignore = env;

    threaded_uct_player rp(0, false);
    slow_tree_uct_player stup(800, true);
    rp.init_pieces();
    stup.init_pieces();

    if (world_comm.rank() == 0) {//master plays the game
        game g(&stup, &rp);
        abstract_player* winner = g.playout();
        if (winner == &stup) {
            cout << "Won" << endl;
        } else if (winner == &rp) {
            cout << "Lost" << endl;
        } else {
            cout << "Draw" << endl;
        }
#ifdef _DEBUG
        g.print_board(true);
#endif
        cout << "Total Simulations: " << stup.get_total_simulations() << " vs " << rp.get_total_simulations() << endl;
    } else {
        stup.do_slave_job();
    }

    return 0;
}
