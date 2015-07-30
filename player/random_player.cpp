#include "random_player.h"
#include <vector>
#include <random>

using namespace std;

random_player::random_player(bool red) :
    abstract_player(red)
{}

random_player::random_player(const abstract_player &b) :
    abstract_player(b)
{}

bool random_player::think_next_move(pos_move &_move, const board &bd, uint8_t, const vector<pos_move> &)
{
    random_device device;
    static thread_local mt19937 generator(device());

    auto all_avail_moves = get_all_available_moves(bd);
    int moves = all_avail_moves.size();
    if (moves == 0) {
        return false;
    }

    uniform_int_distribution<int> m_distribution(0, moves - 1);
    _move = all_avail_moves.at(m_distribution(generator));
    return true;
}
