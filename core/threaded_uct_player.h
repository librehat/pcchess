#ifndef THREADED_UCT_PLAYER_H
#define THREADED_UCT_PLAYER_H

#include "uct_player.h"
#include <vector>
#include <thread>

class threaded_uct_player : public uct_player
{
public:
    threaded_uct_player(double _think_time, bool first_hand, const abstract_player* const _opp, int _threads = 0);

    bool think_next_move(pos_move &_move, const board &m_board);
    void opponent_moved(const pos_move &m);

private:
    int threads;
    std::vector<std::thread> thread_vec;
};

#endif //THREADED_UCT_PLAYER_H
