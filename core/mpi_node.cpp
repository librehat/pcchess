#include "mpi_node.h"

mpi_node::mpi_node(abstract_player *_our, abstract_player *_opp, bool _my_turn, node *_parent) :
    node(_our, _opp, _my_turn, _parent)
{}
