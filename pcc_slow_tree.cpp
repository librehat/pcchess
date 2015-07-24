#include "core/game.h"
#include "player/random_player.h"
#include "player/uct_player.h"
#include "player/threaded_uct_player.h"
#include "player/slow_tree_uct_player.h"
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

    //random_player rp(true);
    threaded_uct_player rp(true);
    slow_tree_uct_player stup(500, false);
    rp.init_pieces();
    stup.init_pieces();

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

        for (int i = 1; i < world_comm.size(); ++i) {
            world_comm.send(i, slow_tree_uct_player::TAG_EXIT);
        }
    } else {
        stup.do_slave_job();
    }

    int sims = stup.get_total_simulations(), sum_sims = 0;
#ifdef _DEBUG
    cout << "[" << world_comm.rank() << "] simulations : " << sims << endl;
#endif
    mpi::reduce(world_comm, sims, sum_sims, plus<int>(), 0);//std::plus is equivalent to MPI_SUM in C
    if (rank == 0) {
        cout << "Total Simulations: " << sum_sims << " v.s. " << rp.get_total_simulations() << endl;
    }

    return 0;
}
