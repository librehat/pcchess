#include "random_player.h"
#include <vector>

using namespace std;

random_player::random_player(board &bd) :
    abstract_player(bd),
    generator(device())
{
}

bool random_player::think_next_move(position &from, position &to)
{
    vector<const abstract_piece*> movable_pieces;
    for (auto it = pieces.begin(); it != pieces.end(); ++it) {
        (*it)->update_moves();
        if ((*it)->is_movable()) {
            movable_pieces.push_back(*it);
        }
    }

    int movables = movable_pieces.size();
    if (movables == 0) {
        return false;
    }

    uniform_int_distribution<int> piece_distribution(0, movables - 1);
    const abstract_piece* piece = movable_pieces.at(piece_distribution(generator));
    const list<position>& avail_moves = piece->get_avail_moves();
    uniform_int_distribution<int> move_distribution(0, avail_moves.size() - 1);
    auto move_it = avail_moves.cbegin();

    for (int random_move = move_distribution(generator); random_move > 0; --random_move, ++move_it);

    from = piece->get_position();
    to   = *move_it;
    return true;
}
