#include "game.h"
#include <iostream>
#include <stdexcept>

using namespace std;

game::game() :
    our_player(nullptr),
    opp_player(nullptr),
    game_over(false)
{
    board = new abstract_piece**[9];
    board_data = new abstract_piece*[90];

    for (int i = 0; i < 90; i++) {
        board_data[i] = nullptr;
    }

    for (int i = 0; i < 9; i++) {
        board[i] = board_data + i * 10;
    }
}

game::~game()
{
    delete [] board;
    delete [] board_data;
    if (our_player) {
        delete our_player;
    }
    if (opp_player) {
        delete opp_player;
    }
}

void game::setup_players(abstract_player *our, abstract_player *opp)
{
    if (our_player || opp_player) {
        throw runtime_error("Error. Players are already initialised.");
    }

    our_player = our;
    opp_player = opp;

    auto our_pieces = our->get_pieces();
    auto opp_pieces = opp->get_pieces();

    for (auto&& it : our_pieces) {//access by reference
        position ipos = it->get_position();
        if(board[ipos.file][ipos.rank]) {
            throw runtime_error("Error. The position in the board is already taken.");
        }
        board[ipos.file][ipos.rank] = it;
    }

    for (auto&& it : opp_pieces) {//access by reference
        position ipos = it->get_position();
        if(board[ipos.file][ipos.rank]) {
            throw runtime_error("Error. The position in the board is already taken.");
        }
        board[ipos.file][ipos.rank] = it;
    }
}

void game::move_piece(const position &from, const position &to)
{
    abstract_piece* piece = board[from.file][from.rank];
    if (!piece){
        cerr << "Error: The piece to move is nullptr on the board." << endl;
        return;
    }

    if (!our_player || ! opp_player) {
        throw runtime_error("Error. Player pointer is NULL!");
    }

    abstract_piece* target = board[to.file][to.rank];
    if (target) {//capture the target
        if (target->is_opposite_side()) {
            opp_player->remove(target);
        } else {
            our_player->remove(target);
        }
    }

    board[from.file][from.rank] = nullptr;
    board[to.file][to.rank] = piece;
    piece->move_to_pos(to);

    game_over = our_player->is_checkmated() || opp_player->is_checkmated();
}

abstract_piece*** game::get_board() const
{
    return board;
}

void game::print_board(bool chinese_char) const
{
    for(int j = 0; j <= 9 ; ++j) {//rank
        for (int i = 0; i <= 8; ++i) {//file
            if (board[i][j]) {
                if (chinese_char) {
                    cout << board[i][j]->chinese_name();
                } else {
                    cout << board[i][j]->abbr_name();
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

bool game::is_over() const
{
    return game_over;
}
