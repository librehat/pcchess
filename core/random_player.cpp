#include "random_player.h"
#include <vector>

using namespace std;

random_player::random_player(board &bd) :
    abstract_player(bd),
    generator(device())
{}

random_player::random_player(const abstract_player &b, board &new_board) :
    abstract_player(b, new_board),
    generator(device())
{}

bool random_player::think_next_move(pos_move &_move)
{
    vector<pos_move> all_avail_moves;
    for (auto it = pieces.begin(); it != pieces.end(); ++it) {
        (*it)->update_moves();
        if ((*it)->is_movable()) {
        	for (auto &&m : (*it)->get_avail_moves()) {
        		pos_move mov;
        		mov[0] = (*it)->get_position();
        		mov[1] = m;
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
