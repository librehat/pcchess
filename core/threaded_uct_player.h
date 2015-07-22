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
    threaded_uct_player(bool opposite = false, int _threads = 0);

    bool think_next_move(pos_move &_move, const board &, const abstract_player &opponent);
    void opponent_moved(const pos_move &m, const abstract_player &opponent);
    int get_total_simulations() const;

private:
    int threads;
    std::vector<std::thread> thread_vec;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(uct_player);
        ar & BOOST_SERIALIZATION_NVP(threads);
    }
};

#endif //THREADED_UCT_PLAYER_H
