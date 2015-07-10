#ifndef MPI_NODE_H
#define MPI_NODE_H

#include "node.h"
#include <boost/mpi.hpp>

class mpi_node : public node
{
public:
    mpi_node(abstract_player* _our, abstract_player* _opp, bool _my_turn, node *_parent = nullptr);
};

#endif //MPI_NODE_H
