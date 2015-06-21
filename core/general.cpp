#include "general.h"

const position general::up = position(0, -1);
const position general::down = position(0, 1);
const position general::left = position(-1, 0);
const position general::right = position(1, 0);

char general::abbr_name() const
{
    return 'G';
}

int general::value() const
{
    return 8;
}

void general::gen_moves(const abstract_piece*** b)
{
    avail_moves.push_back(pos + up);
    avail_moves.push_back(pos + down);
    
    position pos_left = pos + left;
    position pos_right = pos + right;
    
    /* 
     * if there is only one (enemy) general on the left,
     * then our general can't move to the left.
     * the same applies to right.
     */
    bool found_general = false;
    bool other_piece_between = false;
    for (int irank = 0; irank <= 9; irank++) {
        if (b[pos_left.file][irank]) {
            if (found_general) {
                other_piece_between = true;
                break;
            } else if (b[pos_left.file][irank]->abbr_name() == 'G') {
                found_general = true;
            }
        }
    }
    
    if (other_piece_between || !found_general) {
        avail_moves.push_back(pos_left);
    }
    
    found_general = false;
    other_piece_between = false;
    for (int irank = 0; irank <= 9; irank++) {
        if (b[pos_right.file][irank]) {
            if (found_general) {
                other_piece_between = true;
                break;
            } else if (b[pos_right.file][irank]->abbr_name() == 'G') {
                found_general = true;
            }
        }
    }
    
    if (other_piece_between || !found_general) {
        avail_moves.push_back(pos_right);
    }
    
    remove_invalid_moves(3, 5, m_opposite ? 0 : 7, m_opposite ? 2 : 9);
}
