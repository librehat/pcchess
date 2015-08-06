/*
 * threaded_uct_player (together with threaded_node) implements tree-parallelisation with local lock
 */
#ifndef THREADED_UCT_PLAYER_H
#define THREADED_UCT_PLAYER_H

#include "uct_player.h"
#include <vector>
#include <thread>

class threaded_uct_player : public uct_player
{
public:
    explicit threaded_uct_player(int threads = 0, bool red = true);

    bool think_next_move(pos_move &_move, const board &bd, std::uint8_t no_eat_half_rounds, const std::vector<pos_move> &banmoves);

private:
    std::vector<std::thread> thread_vec;
    static std::atomic_bool stop;
    void worker_thread();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(uct_player);
    }
};

#endif //THREADED_UCT_PLAYER_H
