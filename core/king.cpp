#include "king.h"

using namespace std;

const position king::up = position(0, -1);
const position king::down = position(0, 1);
const position king::left = position(-1, 0);
const position king::right = position(1, 0);

abstract_piece* king::make_a_copy() const
{
    return new king(*this);
}

char king::abbr_name() const
{
    return 'K';//use more widespread translation
}

string king::chinese_name() const
{
    return string("將");
}

int king::value() const
{
    return 8;
}

void king::gen_moves(const board &m_board)
{
    avail_moves.push_back(pos + up);
    avail_moves.push_back(pos + down);
    
    position pos_left = pos + left;
    position pos_right = pos + right;
    
    /* 
     * if there is only one (enemy) king on the left,
     * then our king can't move to the left.
     * the same applies to right.
     */
    bool found_king = false;
    bool other_piece_between = false;
    for (int irank = 0; irank <= 9; irank++) {
        if (m_board.at(pos_left.file, irank)) {
            if (found_king) {
                other_piece_between = true;
                break;
            } else if (m_board.at(pos_left.file, irank)->abbr_name() == 'G') {
                found_king = true;
            }
        }
    }
    
    if (other_piece_between || !found_king) {
        avail_moves.push_back(pos_left);
    }
    
    found_king = false;
    other_piece_between = false;
    for (int irank = 0; irank <= 9; irank++) {
        if (m_board.at(pos_right.file, irank)) {
            if (found_king) {
                other_piece_between = true;
                break;
            } else if (m_board.at(pos_right.file, irank)->abbr_name() == 'G') {
                found_king = true;
            }
        }
    }
    
    if (other_piece_between || !found_king) {
        avail_moves.push_back(pos_right);
    }
    
    remove_invalid_moves(m_board, 3, 5, m_opposite ? 0 : 7, m_opposite ? 2 : 9);
}