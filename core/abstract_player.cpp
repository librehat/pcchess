#include "abstract_player.h"
#include "pawn.h"
#include "cannon.h"
#include "chariot.h"
#include "horse.h"
#include "elephant.h"
#include "advisor.h"
#include "king.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>

using namespace std;

abstract_player::abstract_player(bool red) :
    red_side(red),
    checked(false),
    checkmated(false)
{
    pieces.reserve(16);
}

abstract_player::abstract_player(const abstract_player &b) :
    red_side(b.red_side),
    checked(b.checked),
    checkmated(b.checkmated)
{
    pieces.reserve(16);
	for (auto &&it : b.pieces) {
        p_piece p = it->make_a_copy();
        pieces.push_back(p);
	}
#ifdef _DEBUG
    if (pieces.empty()) {
        cout << "New player's pieces vector is empty after copy construction." << endl;
    }
#endif
}

abstract_player::~abstract_player()
{
    for (auto &&p : pieces) {
        delete p;
    }
}

void abstract_player::add(p_piece p)
{
    if(p->is_redside() != red_side) {
        throw invalid_argument("piece to add belongs to different side");
    }
    pieces.push_back(p);
}

void abstract_player::remove(p_piece p)
{
    auto ip = find(pieces.begin(), pieces.end(), p);
    if (ip != pieces.end()) {
        if (p->is_king()) {
            checkmated = true;
        }
        pieces.erase(ip);
        delete p;
    } else {
        throw invalid_argument("piece to remove doesn't belong to this player");
    }
}

void abstract_player::init_pieces()
{
    if (!pieces.empty()) {
        throw runtime_error("pieces vector is not empty when init_pieces is called");
    }

    //pawns
    pieces.push_back(new pawn(0, red_side ? 3 : 6, red_side));
    pieces.push_back(new pawn(2, red_side ? 3 : 6, red_side));
    pieces.push_back(new pawn(4, red_side ? 3 : 6, red_side));
    pieces.push_back(new pawn(6, red_side ? 3 : 6, red_side));
    pieces.push_back(new pawn(8, red_side ? 3 : 6, red_side));

    //cannons
    pieces.push_back(new cannon(1, red_side ? 2 : 7, red_side));
    pieces.push_back(new cannon(7, red_side ? 2 : 7, red_side));

    //chariots
    pieces.push_back(new chariot(0, red_side ? 0 : 9, red_side));
    pieces.push_back(new chariot(8, red_side ? 0 : 9, red_side));

    //horses
    pieces.push_back(new horse(1, red_side ? 0 : 9, red_side));
    pieces.push_back(new horse(7, red_side ? 0 : 9, red_side));

    //elephants
    pieces.push_back(new elephant(2, red_side ? 0 : 9, red_side));
    pieces.push_back(new elephant(6, red_side ? 0 : 9, red_side));

    //advisors
    pieces.push_back(new advisor(3, red_side ? 0 : 9, red_side));
    pieces.push_back(new advisor(5, red_side ? 0 : 9, red_side));

    //king (general)
    pieces.push_back(new king(4, red_side ? 0 : 9, red_side));
}

bool abstract_player::is_redside() const
{
    return red_side;
}

bool abstract_player::is_checked() const
{
    return checked;
}

bool abstract_player::is_checkmated() const
{
    return checkmated;
}

const std::vector<p_piece> &abstract_player::get_pieces() const
{
    return pieces;
}
