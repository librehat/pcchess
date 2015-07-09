#include "abstract_player.h"
#include "soldier.h"
#include "cannon.h"
#include "rook.h"
#include "horse.h"
#include "elephant.h"
#include "advisor.h"
#include "general.h"
#include <stdexcept>
#include <iostream>

using namespace std;

abstract_player::abstract_player() :
    checked(false),
    checkmated(false)
{}

abstract_player::abstract_player(const abstract_player &b) :
    move_history(b.move_history),
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
    pieces.push_back(p);
}

bool abstract_player::remove(p_piece p)
{
    for (auto &&ip : pieces) {
        if (ip == p) {
            if (p->abbr_name() == 'G') {
                checkmated = true;
            }
            pieces.remove(p);
            delete p;
            return true;
        }
    }
#ifdef _DEBUG
    cout << "The piece to remove doesn't belong to this player" << endl;
#endif
    return false;
}

void abstract_player::add_history(const position &from, const position &to)
{
	pos_move m;
	m[0] = from;
	m[1] = to;
	add_history(m);
}

void abstract_player::add_history(const pos_move &m)
{
	move_history.push_front(m);
}

const list<pos_move>& abstract_player::get_history() const
{
	return move_history;
}

void abstract_player::clear_history()
{
    move_history.clear();
}

void abstract_player::init_pieces(bool opposite)
{
    if (!pieces.empty()) {
        throw runtime_error("Error. pieces list is not empty when init_pieces is called.");
    }

    p_piece p;

    //soldiers (pawns)
    p = p_piece(new soldier(0, opposite ? 3 : 6, opposite));
    pieces.push_back(p);
    p = p_piece(new soldier(2, opposite ? 3 : 6, opposite));
    pieces.push_back(p);
    p = p_piece(new soldier(4, opposite ? 3 : 6, opposite));
    pieces.push_back(p);
    p = p_piece(new soldier(6, opposite ? 3 : 6, opposite));
    pieces.push_back(p);
    p = p_piece(new soldier(8, opposite ? 3 : 6, opposite));
    pieces.push_back(p);

    //cannons
    p = p_piece(new cannon(1, opposite ? 2 : 7, opposite));
    pieces.push_back(p);
    p = p_piece(new cannon(7, opposite ? 2 : 7, opposite));
    pieces.push_back(p);

    //rooks
    p = p_piece(new rook(0, opposite ? 0 : 9, opposite));
    pieces.push_back(p);
    p = p_piece(new rook(8, opposite ? 0 : 9, opposite));
    pieces.push_back(p);

    //horses
    p = p_piece(new horse(1, opposite ? 0 : 9, opposite));
    pieces.push_back(p);
    p = p_piece(new horse(7, opposite ? 0 : 9, opposite));
    pieces.push_back(p);

    //elephants
    p = p_piece(new elephant(2, opposite ? 0 : 9, opposite));
    pieces.push_back(p);
    p = p_piece(new elephant(6, opposite ? 0 : 9, opposite));
    pieces.push_back(p);

    //advisors
    p = p_piece(new advisor(3, opposite ? 0 : 9, opposite));
    pieces.push_back(p);
    p = p_piece(new advisor(5, opposite ? 0 : 9, opposite));
    pieces.push_back(p);

    //general
    p = p_piece(new general(4, opposite ? 0 : 9, opposite));
    pieces.push_back(p);
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

