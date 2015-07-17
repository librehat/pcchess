#include "slow_tree_uct_player.h"
#include "../core/game.h"
#include "../core/random_player.h"
#include "../core/serialization_export.h"

using namespace std;
namespace mpi = boost::mpi;

BOOST_CLASS_EXPORT_GUID(slow_tree_uct_player, "slow_tree_uct_player")

int main(int argc, char **argv)
{
    /*
     * the constructor of environment is equivalent to MPI_Init
     * and the destructor is to MPI_Finalize
     */
    mpi::environment env(argc, argv);
    mpi::communicator world_comm;
    int rank = world_comm.rank();

    random_player rp(nullptr, true);
    slow_tree_uct_player stup(2, &rp, false);

    if (rank == 0) {//master plays the game

        game g(&stup, &rp);
        abstract_player* winner = g.playout();
        if (winner == &stup) {
            cout << "Won" << endl;
        } else if (winner == &rp) {
            cout << "Lost" << endl;
        } else {
            cout << "Draw" << endl;
        }
    } else {
        //TODO slaves are dedicated for work (not involved in game playing)
        //
    }

    return 0;
}
