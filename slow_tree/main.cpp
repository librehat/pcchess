#include "slow_tree_uct_player.h"
#include "../core/game.h"
#include "../core/random_player.h"
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
    int rank = world_comm.rank();

    slow_tree_uct_player stup(2, nullptr, false);
    random_player rp(&stup, true);
    stup.set_opponent_player(&rp);

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
    }

    return 0;
}
