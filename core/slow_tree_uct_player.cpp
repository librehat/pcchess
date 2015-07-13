#include "slow_tree_uct_player.h"
#include "random_player.h"
#include "game.h"
#include <boost/mpi.hpp>
#include <stdexcept>

using namespace std;
namespace mpi = boost::mpi;

slow_tree_uct_player::slow_tree_uct_player(double _think_time, const abstract_player * const _opp, bool opposite, int _compute_nodes) :
    uct_player(_think_time, _opp, opposite),
    compute_nodes(_compute_nodes)
{
    if (!mpi::environment::initialized()) {
        throw runtime_error("MPI environment is not initialised");
    }
}

bool slow_tree_uct_player::think_next_move(pos_move &_move, const board &)
{
    if (!root) {
        root = new node(new random_player(*this), new random_player(*opponent), true);
    }
}

void slow_tree_uct_player::opponent_moved(const pos_move &m)
{
    //TODO
}

int slow_tree_uct_player::get_total_simulations() const
{
    //TODO
}
