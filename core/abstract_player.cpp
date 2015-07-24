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

using namespace std;

abstract_player::abstract_player(bool opposite) :
    opposite_player(opposite),
    checked(false),
    checkmated(false)
{}

abstract_player::abstract_player(const abstract_player &b) :
    opposite_player(b.opposite_player),
    checked(b.checked),
    checkmated(b.checkmated)
{
	for (auto &&it : b.pieces) {
        p_piece p = it->make_a_copy();
		pieces.push_back(p);
	}
#ifdef _DEBUG
    if (pieces.empty()) {
        cout << "New player's pieces list is empty after copy construction." << endl;
    }
#endif
}

abstract_player::~abstract_player()
{
	for (auto &&it : pieces) {
		delete it;
	}
}

void abstract_player::add(p_piece p)
{
    if(p->is_opposite_side() != opposite_player) {
        throw invalid_argument("piece to add belongs to different side");
    }
    pieces.push_back(p);
}

void abstract_player::remove(p_piece p)
{
    for (auto &&ip : pieces) {
        if (ip == p) {
            if (p->abbr_name() == 'G') {
                checkmated = true;
            }
            pieces.remove(p);
            delete p;
            return;
        }
    }
    throw invalid_argument("The piece to remove doesn't belong to this player");
}

void abstract_player::init_pieces()
{
    if (!pieces.empty()) {
        throw runtime_error("Error. pieces list is not empty when init_pieces is called.");
    }

    p_piece p;

    //pawns
    p = p_piece(new pawn(0, opposite_player ? 3 : 6, opposite_player));
    pieces.push_back(p);
    p = p_piece(new pawn(2, opposite_player ? 3 : 6, opposite_player));
    pieces.push_back(p);
    p = p_piece(new pawn(4, opposite_player ? 3 : 6, opposite_player));
    pieces.push_back(p);
    p = p_piece(new pawn(6, opposite_player ? 3 : 6, opposite_player));
    pieces.push_back(p);
    p = p_piece(new pawn(8, opposite_player ? 3 : 6, opposite_player));
    pieces.push_back(p);

    //cannons
    p = p_piece(new cannon(1, opposite_player ? 2 : 7, opposite_player));
    pieces.push_back(p);
    p = p_piece(new cannon(7, opposite_player ? 2 : 7, opposite_player));
    pieces.push_back(p);

    //chariots
    p = p_piece(new chariot(0, opposite_player ? 0 : 9, opposite_player));
    pieces.push_back(p);
    p = p_piece(new chariot(8, opposite_player ? 0 : 9, opposite_player));
    pieces.push_back(p);

    //horses
    p = p_piece(new horse(1, opposite_player ? 0 : 9, opposite_player));
    pieces.push_back(p);
    p = p_piece(new horse(7, opposite_player ? 0 : 9, opposite_player));
    pieces.push_back(p);

    //elephants
    p = p_piece(new elephant(2, opposite_player ? 0 : 9, opposite_player));
    pieces.push_back(p);
    p = p_piece(new elephant(6, opposite_player ? 0 : 9, opposite_player));
    pieces.push_back(p);

    //advisors
    p = p_piece(new advisor(3, opposite_player ? 0 : 9, opposite_player));
    pieces.push_back(p);
    p = p_piece(new advisor(5, opposite_player ? 0 : 9, opposite_player));
    pieces.push_back(p);

    //king (general)
    p = p_piece(new king(4, opposite_player ? 0 : 9, opposite_player));
    pieces.push_back(p);
}

bool abstract_player::is_opposite() const
{
    return opposite_player;
}

bool abstract_player::is_checked() const
{
    return checked;
}

bool abstract_player::is_checkmated() const
{
    return checkmated;
}

const list<p_piece>& abstract_player::get_pieces() const
{
    return pieces;
}
