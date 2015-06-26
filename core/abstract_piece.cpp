#include "abstract_piece.h"

using namespace std;

abstract_piece::abstract_piece(int _file, int _rank, bool oppo, board &_board) :
    m_board(_board),
    m_opposite(oppo),
    pos(_file, _rank)
{}

const bool abstract_piece::is_opposite_side() const
{
    return m_opposite;
}

position abstract_piece::get_position() const
{
    return pos;
}

void abstract_piece::move_to_pos(int newfile, int newrank)
{
    pos.file = newfile;
    pos.rank = newrank;
}

void abstract_piece::move_to_pos(const position &new_pos)
{
    pos = new_pos;
}

void abstract_piece::update_moves()
{
    avail_moves.clear();
    if (can_i_move()) {
        gen_moves();
    }
}

bool abstract_piece::can_i_move() const
{
    int pieces_in_between = 0;
    bool found_one_g = false;
    bool am_i_in_between = false;
    for (int irank = 0; irank <= 9; irank++) {
        const p_piece piece = m_board.at(pos.file, irank);
        if (piece) {
            if (found_one_g) {
                if (piece->abbr_name() == 'G') {//another General
                    break;
                }
                
                pieces_in_between++;
                if (piece == this) {
                    am_i_in_between = true;
                }
            } else if (piece->abbr_name() == 'G') {
                found_one_g = true;
            }
        }
    }
    
    return !am_i_in_between || pieces_in_between != 1;
}

void abstract_piece::remove_invalid_moves(int min_file, int max_file, int min_rank, int max_rank)
{
    for (auto it = avail_moves.begin(); it != avail_moves.end();) {
        bool invalid = false;
        if (it->not_in_range(min_file, max_file, min_rank, max_rank)) {
            invalid = true;
        } else {
            auto target_piece = m_board[*it];
            if (target_piece) {
                //can't capture same-side pieces
                if (target_piece->is_opposite_side() == this->m_opposite) {
                    invalid = true;
                }
            }
        }

        if (invalid) {
            it = avail_moves.erase(it);
        } else {
            ++it;
        }
    }
}

const list<position>& abstract_piece::get_avail_moves() const
{
    return avail_moves;
}

bool abstract_piece::is_movable() const
{
    return !avail_moves.empty();
}
