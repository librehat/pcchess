#include "core/game.h"
#include "player/random_player.h"
#include "player/uct_player.h"
#include "player/threaded_uct_player.h"
#include "player/root_uct_player.h"
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

    root_uct_player tr(true);
    //threaded_uct_player tb(0, false);
    random_player tb(false);
    tr.init_pieces();
    tb.init_pieces();

    if (world_comm.rank() == 0) {//master plays the game
        game g(&tr, &tb);
        abstract_player* winner = g.playout();
        if (winner == &tr) {
            cout << "Won" << endl;
        } else if (winner == &tb) {
            cout << "Lost" << endl;
        } else {
            cout << "Draw" << endl;
        }
#ifdef _DEBUG
        g.print_board(true);
#endif
        cout << "Total Simulations: " << tr.get_total_simulations() << " vs " << tb.get_total_simulations() << endl;
    } else {
        tr.do_slave_job();
    }

    return 0;
}
