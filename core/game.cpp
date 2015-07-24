#include "game.h"
#include <iostream>
#include <stdexcept>

using namespace std;

game::game(abstract_player* _red, abstract_player* _black, unsigned int not_eat_rounds) :
    red(_red),
    black(_black),
    rounds_since_last_eat(not_eat_rounds)
{
    if (red->is_opposite() || !black->is_opposite()) {
        throw invalid_argument("player is in the wrong side");
    } else {
        setup_players();
    }
    history.reserve(200);
}

long int game::step_time = 2000;
unsigned int game::NO_EAT_DRAW_ROUNDS = 60;

game::~game()
{}

abstract_player* game::playout(bool red_first)
{
    pos_move next_move;
    bool movable = false;

    abstract_player* first = red_first ? red : black;
    abstract_player* second = red_first ? black : red;

    do {
        rounds_since_last_eat++;
        movable = first->think_next_move(next_move, m_board, *second);
        if (!movable || first->is_checkmated()) {
            return second;
        } else {
            move_piece(next_move);
            first->add_history(next_move);
            second->opponent_moved(next_move, *first);
        }
        movable = second->think_next_move(next_move, m_board, *first);
        if (!movable || second->is_checkmated()) {
            return first;
        } else {
            move_piece(next_move);
            second->add_history(next_move);
            first->opponent_moved(next_move, *second);
        }
    } while (rounds_since_last_eat < NO_EAT_DRAW_ROUNDS && NO_EAT_DRAW_ROUNDS != 0);

    return nullptr;
}

void game::setup_players()
{
    auto our_pieces = red->get_pieces();
    auto opp_pieces = black->get_pieces();

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
    move_piece(pos_move(from, to));
}

void game::move_piece(const pos_move &_move)
{
    p_piece piece = m_board[_move.from];
    if (!piece){
        throw runtime_error("Error. The piece to move is nullptr on the board.");
    }

    p_piece target = m_board[_move.to];
    if (target) {//capture the target
        if (target->is_opposite_side()) {//black side is always the opposite
            black->remove(target);
        } else {
            red->remove(target);
        }
        rounds_since_last_eat = 0;
    }

    m_board[_move.from] = nullptr;
    m_board[_move.to] = piece;
    piece->move_to_pos(_move.to);
    history.push_back(_move);
}

void game::parse_fen(const string &fen)
{
    //TODO
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
