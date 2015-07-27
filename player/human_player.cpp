#include "human_player.h"
#include <algorithm>

using namespace std;

bool human_player::think_next_move(pos_move &m, const board &bd, int8_t no_eat_half_rounds, const std::vector<pos_move> &)
{
    bd.print_out(true);
    vector<pos_move> all_avail_moves;
    for (auto &&p : pieces) {
        p->update_moves(bd);
        if (p->is_movable()) {
            auto from = p->get_position();
            for (auto &&am : p->get_avail_moves()) {
                all_avail_moves.emplace_back(from, am);
            }
        }
    }

    string move_str;
    cout << "==> It has been " << no_eat_half_rounds / 2 << " rounds without any pieces eaten." << endl
         << "==> Please input your next move using ICCI notation (i.e. a0a1)." << endl
         << "==> If you want to resign, please input r." << endl
         << "--> ";
    cin >> move_str;
    if (move_str.compare("r") == 0) {
        return false;
    }

    pos_move proposal(move_str);
    while (find(all_avail_moves.begin(), all_avail_moves.end(), proposal) == all_avail_moves.end()) {
        cout << "==> You can't make such a move, please try again" << endl
             << "--> ";
        cin >> move_str;
        if (move_str.compare("r") == 0) {
            return false;
        }
        proposal = pos_move(move_str);
    }

    m = pos_move(move_str);
    return true;
}
