#include "abstract_player.h"

using namespace std;

abstract_player::abstract_player()
{
}

abstract_player::~abstract_player()
{
    for (auto it = pieces.begin(); it != pieces.end(); ++it) {
        delete *it;
    }
}

void abstract_player::add(abstract_piece *p)
{
    pieces.push_back(p);
}

void abstract_player::remove(abstract_piece *p)
{
    pieces.remove(p);
    delete p;
}
