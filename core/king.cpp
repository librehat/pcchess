#include "king.h"

using namespace std;

const position king::up = position(0, 1);
const position king::down = position(0, -1);
const position king::left = position(-1, 0);
const position king::right = position(1, 0);

abstract_piece* king::make_a_copy() const
{
    return new king(*this);
}

char king::abbr_name() const
{
    return red_side ? 'K' : 'k';//use more widespread translation
}

string king::print_name(const bool &chinese) const
{
    return red_side ? chinese ? string("帥") : string("Ｋ")
                    : chinese ? string("將") : string("ｋ");
}

int king::value() const
{
    return 8;
}

bool king::is_flying_king(const position &proposed, const board &m_board) const
{
    int8_t king_rank = -1;

    for (int8_t irank = 0; irank < board::RANK_NUM; irank++) {
        auto p = m_board.at(proposed.file, irank);
        if (p) {
            if (p->is_king()) {
                king_rank = irank;
                break;
            }
        }
    }

    if (king_rank == -1) {//there is no king, don't bother the flying king rule
        return false;
    }

    bool triggered = true;

    if (red_side) {
        for (int8_t irank = proposed.rank + 1; irank < king_rank; irank++) {//proposed.rank is equal to current position's rank
            if (m_board.at(proposed.file, irank)) {
                triggered = false;
                break;
            }
        }
    } else {
        for (int8_t irank = proposed.rank - 1; irank > king_rank; irank--) {
            if (m_board.at(proposed.file, irank)) {
                triggered = false;
                break;
            }
        }
    }

    return triggered;
}

void king::gen_moves(const board &m_board)
{
    avail_pos.push_back(pos + up);
    avail_pos.push_back(pos + down);
    
    position pos_left = pos + left;
    position pos_right = pos + right;
    
    
    if (!is_flying_king(pos_left, m_board)) {
        avail_pos.push_back(pos_left);
    }
    
    if (!is_flying_king(pos_right, m_board)) {
        avail_pos.push_back(pos_right);
    }
    
    remove_invalid_moves(m_board, 3, 5, red_side ? 0 : 7, red_side ? 2 : 9);
}
