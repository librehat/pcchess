#ifndef THREADED_UCT_PLAYER_H
#define THREADED_UCT_PLAYER_H

#include "uct_player.h"
#include <vector>
#include <thread>

class threaded_uct_player : public uct_player
{
public:
    threaded_uct_player(const abstract_player* const _opp = nullptr, bool opposite = false, int _threads = 0);

    bool think_next_move(pos_move &_move, const board &);
    void opponent_moved(const pos_move &m);
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
