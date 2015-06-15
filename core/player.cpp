#include "player.h"
#include "advisor.h"
#include "cannon.h"
#include "elephant.h"
#include "general.h"
#include "horse.h"
#include "rook.h"
#include "soldier.h"

using namespace std;

player::player()
{
    for (int i = 0; i < 5; ++i) {
        piece_list.push_back(new soldier());
    }
    piece_list.push_back(new cannon());
    piece_list.push_back(new cannon());
    piece_list.push_back(new rook());
    piece_list.push_back(new rook());
    piece_list.push_back(new horse());
    piece_list.push_back(new horse());
    piece_list.push_back(new elephant());
    piece_list.push_back(new elephant());
    piece_list.push_back(new advisor());
    piece_list.push_back(new advisor());
    piece_list.push_back(new general());
}

player::~player()
{
    for (auto it = piece_list.begin(); it != piece_list.end(); ++it) {
        delete *it;
    }
}
