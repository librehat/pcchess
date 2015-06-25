#include "board.h"
#include <stdexcept>

using namespace std;

board::board()
{
    data = new p_piece*[9];
    data_2 = new p_piece[90];

    for (int i = 0; i < 90; i++) {
        data_2[i] = nullptr;
    }

    for (int i = 0; i < 9; i++) {
        data[i] = data_2 + i * 10;
    }
}

board::~board()
{
    delete [] data;
    delete [] data_2;
}

p_piece** &board::data_ref()
{
    return data;
}

p_piece board::at(const int &file, const int &rank) const
{
    int dis = file * 10 + rank;
    if (dis >= 90) {
        throw runtime_error("Error. Accessing an element in board out of range.");
    }
    return data_2[dis];
}
