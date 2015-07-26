#include "abstract_piece.h"

using namespace std;

abstract_piece::abstract_piece(int8_t _file, int8_t _rank, bool red) :
    red_side(red),
    pos(_file, _rank)
{}

abstract_piece::abstract_piece(const abstract_piece &b) :
    red_side(b.red_side),
    pos(b.pos),
    avail_moves(b.avail_moves)
{}

abstract_piece::~abstract_piece()
{}

bool abstract_piece::is_redside() const
{
    return red_side;
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

void abstract_piece::update_moves(const board &m_board)
{
    avail_moves.clear();
    if (can_i_move(m_board)) {
        gen_moves(m_board);
    }
}

bool abstract_piece::can_i_move(const board &m_board) const
{
    int8_t pieces_in_between = 0;
    bool found_one_g = false;
    bool am_i_in_between = false;
    for (int8_t irank = 0; irank < board::RANK_NUM; irank++) {
        auto piece = m_board.at(pos.file, irank);
        if (piece) {
            if (found_one_g) {
                if (piece->is_king()) {
                    break;
                }
                
                pieces_in_between++;
                if (*piece == *this) {
                    am_i_in_between = true;
                }
            } else if (piece->is_king()) {
                found_one_g = true;
            }
        }
    }
    
    return !am_i_in_between || pieces_in_between != 1;
}

void abstract_piece::remove_invalid_moves(const board &m_board, int8_t min_file, int8_t max_file, int8_t min_rank, int8_t max_rank)
{
    for (auto it = avail_moves.begin(); it != avail_moves.end();) {
        bool invalid = false;
        if (it->not_in_range(min_file, max_file, min_rank, max_rank)) {
            invalid = true;
        } else {
            auto target_piece = m_board.at(*it);
            if (target_piece) {
                //can't capture same-side pieces
                if (target_piece->red_side == this->red_side) {
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

const vector<position>& abstract_piece::get_avail_moves() const
{
    return avail_moves;
}

bool abstract_piece::is_movable() const
{
    return !avail_moves.empty();
}

bool abstract_piece::operator ==(const abstract_piece &b)
{
    return !(*this != b);
}

bool abstract_piece::operator !=(const abstract_piece &b)
{
    return red_side != b.red_side && pos != b.pos && avail_moves != b.avail_moves;
}
