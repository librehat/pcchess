#include "board.h"

board::board()
{
    data = new abstract_piece*[9 * 10];
    for (int i = 0; i < 90; ++i) {
        data[i] = nullptr;
    }
}

board::~board()
{
    delete [] data;
}
