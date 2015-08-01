#include "core/game.h"
#include "player/random_player.h"
#include "player/uct_treesplit_player.h"
#include <boost/mpi.hpp>

using namespace std;
namespace mpi = boost::mpi;

int main(int argc, char **argv)
{
    /*
     * at least serialized level. multiple is recommended
     */
    mpi::environment env(argc, argv, mpi::threading::serialized);
    mpi::communicator world_comm;
    std::ignore = env;

    uct_treesplit_player red(4, true);
    random_player black(false);
    red.init_pieces();
    black.init_pieces();

    if (world_comm.rank() == 0) {//master plays the game
        game g(&red, &black);
        abstract_player* winner = g.playout();
        if (winner == &red) {
            cout << "Won" << endl;
        } else if (winner == &black) {
            cout << "Lost" << endl;
        } else {
            cout << "Draw" << endl;
        }
#ifdef _DEBUG
        g.print_board(true);
#endif
        cout << "Total Simulations: " << red.get_total_simulations() << " vs " << black.get_total_simulations() << endl;
    } else {
        red.do_slave_job();
    }

    return 0;
}
