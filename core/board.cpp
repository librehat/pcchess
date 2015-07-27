#include "board.h"
#include "abstract_piece.h"
#include <stdexcept>
#include <iostream>

using namespace std;

board::board()
{
    for (int i = 0; i < NUM; i++) {
        data[i] = nullptr;
    }
}

p_piece* board::operator [](const int8_t &file)
{
    if (file >= FILE_NUM || file < 0) {
        throw std::out_of_range("board file index out of range");
    }
    return data.data() + file * RANK_NUM;
}

p_piece& board::operator [] (const position &pos)
{
    if (pos.not_in_range(0, FILE_NUM - 1, 0, RANK_NUM - 1)) {
        throw std::out_of_range("board position index out of range");
    }
    return data[pos.file * RANK_NUM + pos.rank];
}

p_piece board::at(const int8_t &file, const int8_t &rank) const
{
    if (file < 0 || file >= FILE_NUM || rank < 0 || rank >= RANK_NUM) {
        return nullptr;
    }

    return data[file * RANK_NUM + rank];
}

p_piece board::at(const position &pos) const
{
    return this->at(pos.file, pos.rank);
}

bool board::operator !=(const board &b)
{
    return data != b.data;
}

void board::print_out(bool chinese_char) const
{
    for(int8_t j = board::RANK_NUM - 1; j >= 0 ; --j) {//rank
        cout << to_string(j) << " ";
        for (int8_t i = 0; i < board::FILE_NUM; ++i) {//file
            if (at(i, j)) {
                if (chinese_char) {
                    cout << at(i, j)->chinese_name();
                } else {
                    cout << at(i, j)->abbr_name();
                }
            } else {
                cout << (chinese_char ? "＋" : "+");
            }
            if (i != board::FILE_NUM - 1) {
                cout << (chinese_char ? "－" : "-");
            }
        }
        if (j != 0) {
            if (chinese_char) {
                cout << "\n　｜　｜　｜　｜　｜　｜　｜　｜　｜\n";
            } else {
                cout << "\n  | | | | | | | | |\n";
            }
        }
    }
    cout << "\n  ";
    for (int8_t i = 0; i < board::FILE_NUM; ++i) {
        if (chinese_char) {
            cout << get_full_width_letter(static_cast<char>(i + 'a')) << "　";
        } else {
            cout << static_cast<char>(i + 'a') << ' ';
        }
    }
    cout << endl;
}

string board::get_full_width_letter(const char &a)
{
    int8_t del = a - 'a';
    switch (del) {
    case 0:
        return string("ａ");
    case 1:
        return string("ｂ");
    case 2:
        return string("ｃ");
    case 3:
        return string("ｄ");
    case 4:
        return string("ｅ");
    case 5:
        return string("ｆ");
    case 6:
        return string("ｇ");
    case 7:
        return string("ｈ");
    case 8:
        return string("ｉ");
    case 9:
        return string("ｊ");
    case 10:
        return string("ｋ");
    default:
        return string("??");
    }
}
