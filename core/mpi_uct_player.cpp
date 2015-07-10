#include "mpi_uct_player.h"
#include "mpi_node.h"
#include "random_player.h"
#include "game.h"

using namespace std;

mpi_uct_player::mpi_uct_player(double _think_time, const abstract_player * const _opp, bool opposite, int _compute_nodes) :
    uct_player(_think_time, _opp, opposite),
    compute_nodes(_compute_nodes)
{}

bool mpi_uct_player::think_next_move(pos_move &_move, const board &)
{
    if (!root) {
        root = new mpi_node(new random_player(*this), new random_player(*opponent), true);
    }
}

void mpi_uct_player::opponent_moved(const pos_move &m)
{
    //TODO
}

int mpi_uct_player::get_total_simulations() const
{
    //TODO
}
