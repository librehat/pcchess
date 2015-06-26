#include "game.h"
#include <iostream>
#include <stdexcept>

using namespace std;

game::game(abstract_player* our, abstract_player* opp, board &_board) :
    m_board(_board),
    our_player(our),
    opp_player(opp)
{
    setup_players();
}

game::~game()
{}

abstract_player* game::playout()
{
    pos_move next_move;
    bool movable = false;

    for (int i = 0; i < 200; ++i) {//FIXME: implement the real draw rule
        movable = our_player->think_next_move(next_move);
        if (!movable || our_player->is_checkmated()) {
            return opp_player;
        } else {
            move_piece(next_move);
        }
        movable = opp_player->think_next_move(next_move);
        if (!movable || opp_player->is_checkmated()) {
            return our_player;
        } else {
            move_piece(next_move);
        }
    }

    return nullptr;
}

void game::setup_players()
{
    auto our_pieces = our_player->get_pieces();
    auto opp_pieces = opp_player->get_pieces();

    for (auto&& it : our_pieces) {//access by reference
        position ipos = it->get_position();
        if(m_board[ipos]) {
            throw runtime_error("Error. The position in the board is already taken.");
        }
        m_board[ipos] = it;
    }

    for (auto&& it : opp_pieces) {//access by reference
        position ipos = it->get_position();
        if(m_board[ipos]) {
            throw runtime_error("Error. The position in the board is already taken.");
        }
        m_board[ipos] = it;
    }
}

void game::move_piece(const position &from, const position &to)
{
    abstract_piece* piece = m_board[from];
    if (!piece){
        throw runtime_error("Error. The piece to move is nullptr on the board.");
    }

    abstract_piece* target = m_board[to];
    if (target) {//capture the target
        if (target->is_opposite_side()) {
            opp_player->remove(target);
        } else {
            our_player->remove(target);
        }
    }

    m_board[from] = nullptr;
    m_board[to] = piece;
    piece->move_to_pos(to);
}

void game::move_piece(const pos_move &_move)
{
    move_piece(_move[0], _move[1]);
}

void game::print_board(bool chinese_char) const
{
    for(int j = 0; j <= 9 ; ++j) {//rank
        for (int i = 0; i <= 8; ++i) {//file
            if (m_board.at(i, j)) {
                if (chinese_char) {
                    cout << m_board.at(i, j)->chinese_name();
                } else {
                    cout << m_board.at(i, j)->abbr_name();
                }
            } else {
                cout << (chinese_char ? "＋" : "+");
            }
            if (i != 8) {
                cout << (chinese_char ? "－" : "-");
            }
        }
        if (j != 9) {
            if (chinese_char) {
                cout << "\n｜　｜　｜　｜　｜　｜　｜　｜　｜\n";
            } else {
                cout << "\n| | | | | | | | |\n";
            }
        }
    }
    cout << endl;
}
