#include "game.h"
#include "king.h"
#include "pawn.h"
#include "elephant.h"
#include "adviser.h"
#include "chariot.h"
#include "horse.h"
#include "cannon.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;

game::game(abstract_player* _red, abstract_player* _black, uint8_t no_eat_half_rounds) :
    red(_red),
    black(_black),
    half_rounds_since_last_eat(no_eat_half_rounds),
    rounds(0)
{
    if (!red->is_redside() || black->is_redside()) {
        throw invalid_argument("player is in the wrong side");
    } else {
        setup_players();
    }
}

game::game(abstract_player *_red, abstract_player *_black, const string &fen, uint8_t no_eat_half_rounds) :
    red(_red),
    black(_black),
    half_rounds_since_last_eat(no_eat_half_rounds),
    rounds(0)
{
    if (!red->is_redside() || black->is_redside()) {
        throw invalid_argument("player is in the wrong side");
    } else {
        parse_fen(fen);
    }
}

long int game::step_time = 1000;
uint8_t game::NO_EAT_DRAW_HALF_ROUNDS = 120;

abstract_player* game::playout(bool red_first)
{
    /*
     * "validate" the game state before start
     * this could prevent game from useless playing since one side is already lost before start
     */
    if (!red->get_king()) {
        return black;
    } else if (!black->get_king()) {
        return red;
    }

    pos_move next_move;
    bool movable = false;

    auto& first = red_first ? red : black;
    auto& second = red_first ? black : red;
    vector<pos_move> &first_banmoves = red_first ? red_banmoves : black_banmoves;
    vector<pos_move> &second_banmoves = red_first ? black_banmoves : red_banmoves;

    for (int i = 0; i < numeric_limits<int>::max(); ++i) {
        rounds++;

        first->set_check(is_player_in_check(red_first));
        movable = first->think_next_move(next_move, m_board, half_rounds_since_last_eat, first_banmoves);
        if (!movable) {
            return second;
        } else {
            if (find(first_banmoves.begin(), first_banmoves.end(), next_move) != first_banmoves.end()) {
                return second;
            }
            move_piece(next_move);
            if (second->is_checkmated()) {
                return first;
            }
            second->opponent_moved(next_move);
        }

        second->set_check(is_player_in_check(!red_first));
        movable = second->think_next_move(next_move, m_board, half_rounds_since_last_eat, second_banmoves);
        if (!movable) {
            return first;
        } else {
            if (find(second_banmoves.begin(), second_banmoves.end(), next_move) != second_banmoves.end()) {
                return first;
            }
            move_piece(next_move);
            if (first->is_checkmated()) {
                return second;
            }
            first->opponent_moved(next_move);
        }

        //TODO banmoves should be updated after one round
        first_banmoves.clear();
        second_banmoves.clear();

        if (half_rounds_since_last_eat >= NO_EAT_DRAW_HALF_ROUNDS && NO_EAT_DRAW_HALF_ROUNDS != 0) {
            return nullptr;
        }
    }

    throw runtime_error("reached the limit of game rounds. this game is endless.");
}

void game::setup_players()
{
    auto red_pieces = red->get_pieces();
    auto black_pieces = black->get_pieces();

    for (auto &it : red_pieces) {//access by reference
        position ipos = it->get_position();
        if(m_board[ipos]) {
            throw runtime_error("Error. The position in the board is already taken.");
        }
        m_board[ipos] = it;
    }

    for (auto &it : black_pieces) {//access by reference
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
    auto piece = m_board[_move.from];
    if (!piece){
        cerr << "the pos_move " << _move << " is not valid" << endl;
        throw invalid_argument("the piece to move is nullptr on the board");
    }

    auto target = m_board[_move.to];
    if (target) {//capture the target
        if (target->is_redside()) {
            red->remove(target);
        } else {
            black->remove(target);
        }
        half_rounds_since_last_eat = 0;
    } else {
        half_rounds_since_last_eat++;
    }

    m_board[_move.from] = nullptr;
    m_board[_move.to] = piece;
    piece->move_to_pos(_move.to);
    history.push_front(_move);
}

void game::set_red_banmoves(const std::vector<pos_move> &bm)
{
    red_banmoves = bm;
}

void game::set_black_banmoves(const std::vector<pos_move> &bm)
{
    black_banmoves = bm;
}

const vector<pos_move>& game::get_red_banmoves() const
{
    return red_banmoves;
}

const vector<pos_move>& game::get_black_banmoves() const
{
    return black_banmoves;
}

void game::parse_fen(const string &fen)
{
    if (!red->get_pieces().empty() || !black->get_pieces().empty()) {
        cerr << "player's pieces is not empty before calling parse_fen" << endl;
    }

    vector<string> rank_str;
    boost::split(rank_str, fen, boost::is_any_of("/"), boost::token_compress_off);

    if (rank_str.size() != 10) {
        cerr << "invalid FEN string: " << fen << endl;
        return;
    }

    /*
     * the first line is rank 9, then 8... the order is reversed
     * so we firstly reverse this vector in order to be straightforward
     */
    reverse(rank_str.begin(), rank_str.end());

    for (int8_t rank = 0; rank < board::RANK_NUM; ++rank) {
        const string &str = rank_str[rank];
        int8_t file = 0;
        char c;
        for (std::size_t i = 0; i < str.size(); ++i) {
            c = str[i];
            if (c <= '9') {
                file += static_cast<int>(c - '0');
            } else {
                p_piece p;
                switch (c) {//upper-case: red side; lower-case: black side
                case 'K':
                    p = new king(file, rank, true);
                    break;
                case 'k':
                    p = new king(file, rank, false);
                    break;
                case 'P':
                    p = new pawn(file, rank, true);
                    break;
                case 'p':
                    p = new pawn(file, rank, false);
                    break;
                case 'B':
                    p = new elephant(file, rank, true);
                    break;
                case 'b':
                    p = new elephant(file, rank, false);
                    break;
                case 'A':
                    p = new adviser(file, rank, true);
                    break;
                case 'a':
                    p = new adviser(file, rank, false);
                    break;
                case 'R':
                    p = new chariot(file, rank, true);
                    break;
                case 'r':
                    p = new chariot(file, rank, false);
                    break;
                case 'N':
                    p = new horse(file, rank, true);
                    break;
                case 'n':
                    p = new horse(file, rank, false);
                    break;
                case 'C':
                    p = new cannon(file, rank, true);
                    break;
                case 'c':
                    p = new cannon(file, rank, false);
                    break;
                default:
                    cerr << "Unknown character in FEN string: " << c;
                    throw invalid_argument("invalid FEN string");
                }
                if (p->is_redside()) {
                    red->add(p);
                } else {
                    black->add(p);
                }
                file++;
            }
        }
        if (file != 9) {
            cerr << "Current field's FEN string is incorrect!" << endl;
        }
    }
    setup_players();
}

string game::get_fen() const
{
    return generate_fen(m_board);
}

string game::generate_fen(const board &bd)
{
    std::string fen;
    int8_t space = 0;
    for (int8_t rank = board::RANK_NUM - 1; rank >= 0; --rank) {
        for (int8_t file = 0; file < board::FILE_NUM; ++file) {
            auto p = bd.at(file, rank);
            if (p) {
                if (space != 0) {
                    fen.push_back(static_cast<char>(space) + '0');
                    space = 0;
                }
                fen.push_back(p->abbr_name());
            } else {
                space++;
            }
        }
        if (space != 0) {
            fen.push_back(static_cast<char>(space) + '0');
            space = 0;
        }
        fen.push_back('/');
    }
    fen.pop_back();//remove last '/'
    return fen;
}

const std::deque<pos_move> &game::get_history() const
{
    return history;
}

uint8_t game::get_half_rounds_since_last_eat() const
{
    return half_rounds_since_last_eat;
}

const int &game::get_rounds() const
{
    return rounds;
}

void game::print_board(bool chinese_char) const
{
    m_board.print_out(chinese_char);
}

bool game::is_player_in_check(const bool &_red)
{
    auto &we = _red ? red : black;
    auto &op = _red ? black : red;
    auto op_avail_moves = op->get_all_available_target_positions(m_board);
    auto king_pos = we->get_king_position();
    return find(op_avail_moves.begin(), op_avail_moves.end(), king_pos) != op_avail_moves.end();
}
