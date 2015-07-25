#include "random_player.h"
#include <vector>
#include <random>

using namespace std;

random_player::random_player(bool opposite) :
    abstract_player(opposite)
{}

random_player::random_player(const abstract_player &b) :
    abstract_player(b)
{}

bool random_player::think_next_move(pos_move &_move, const board &bd, const abstract_player &, unsigned int, const vector<pos_move> &)
{
    random_device device;
    static thread_local mt19937 generator(device());

    vector<pos_move> all_avail_moves;
    for (auto &&p : pieces) {
        p->update_moves(bd);
        if (p->is_movable()) {
            pos_move mov;
            mov.from = p->get_position();
        	for (auto &&m : p->get_avail_moves()) {
                mov.to = m;
        		all_avail_moves.push_back(mov);
        	}
        }
    }

    int moves = all_avail_moves.size();
    if (moves == 0) {
        return false;
    }

    uniform_int_distribution<int> m_distribution(0, moves - 1);
    _move = all_avail_moves.at(m_distribution(generator));
    return true;
}
