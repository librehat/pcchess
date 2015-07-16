#ifndef THREADED_UCT_PLAYER_H
#define THREADED_UCT_PLAYER_H

#include "uct_player.h"
#include <vector>
#include <thread>

class threaded_uct_player : public uct_player
{
public:
    threaded_uct_player(double _think_time, const abstract_player* const _opp, bool opposite, int _threads = 0);

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
        ar & boost::serialization::base_object<uct_player>(*this);
        ar & threads;
    }
};

#endif //THREADED_UCT_PLAYER_H
