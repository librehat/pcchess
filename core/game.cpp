#include "game.h"
#include <iostream>

using namespace std;

game::game()
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
    for (auto it = our_alive_pieces.begin(); it != our_alive_pieces.end(); ++it) {
        delete *it;
    }
    for (auto it = opp_alive_pieces.begin(); it != opp_alive_pieces.end(); ++it) {
        delete *it;
    }
}

void game::move_piece(const position &from, const position &to)
{
    abstract_piece* piece = board[from.file][from.rank];
    if (!piece){
        cerr << "Error: The piece to move is nullptr on the board." << endl;
        return;
    }

    abstract_piece* target = board[to.file][to.rank];
    if (target) {//capture the target
        if (target->is_opposite_side()) {
            opp_alive_pieces.remove(target);
        } else {
            our_alive_pieces.remove(target);
        }
        delete target;
    }

    board[from.file][from.rank] = nullptr;
    board[to.file][to.rank] = piece;
    piece->move_to_pos(to);
}
