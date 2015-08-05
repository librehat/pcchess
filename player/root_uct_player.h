//root parallelisation (easiest master-slave model without communications in the process)
#ifndef ROOT_UCT_PLAYER_H
#define ROOT_UCT_PLAYER_H

#include "uct_player.h"
#include <boost/mpi/communicator.hpp>

class root_uct_player : public uct_player
{
public:
    explicit root_uct_player(bool red = true);
    ~root_uct_player();

    bool think_next_move(pos_move &_move, const board &bd, std::uint8_t no_eat_half_rounds, const std::vector<pos_move> &banmoves);
    std::int64_t get_total_simulations() const;

    virtual void do_slave_job();

    enum TAG : int { GATHER_TREE, COMP_LOOP, COMP_FINISH, BROADCAST_TREE, REDUCE_SIMS, SYNC, OPP_MOVE, CHILD_SELEC, TS_INIT, TS_MSG, TS_START, TS_STOP, TS_BEST_CHILD, EXIT };

protected:
    void master_send_order(const TAG &tag) const;

    void slave_broadcast_tree();

    static boost::mpi::communicator world_comm;

private:
    void upload_tree();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(uct_player);
    }
};

#endif //ROOT_UCT_PLAYER_H
