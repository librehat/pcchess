#include "slow_tree_uct_player.h"
#include "../core/random_player.h"
#include "../core/game.h"
#include <stdexcept>

using namespace std;
namespace mpi = boost::mpi;

slow_tree_uct_player::slow_tree_uct_player(double _think_time, const abstract_player * const _opp, bool opposite) :
    uct_player(_think_time, _opp, opposite)
{}

mpi::communicator slow_tree_uct_player::world_comm;

bool slow_tree_uct_player::think_next_move(pos_move &_move, const board &)
{
    if (!root) {
        root = new node(new random_player(*this), new random_player(*opponent), true);
    }

    //TODO let slaves do computations with synchronisations (at a given frequency)
    //then do a synchronisation when timed out then gets move from any one of them

    return false;
}

void slow_tree_uct_player::opponent_moved(const pos_move &m)
{
    //TODO broadcasts the move to all slaves
}

int slow_tree_uct_player::get_total_simulations() const
{
    //TODO
}
