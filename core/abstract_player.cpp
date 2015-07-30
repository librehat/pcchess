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
    in_check(false),
    checkmated(false),
    pking(nullptr)
{
    pieces.reserve(16);
}

abstract_player::abstract_player(const abstract_player &b) :
    red_side(b.red_side),
    in_check(b.in_check),
    checkmated(b.checkmated),
    pking(nullptr)
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
    if (p->is_redside() != red_side) {
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

p_piece abstract_player::get_king()
{
    if (!pking) {
        auto kit = find_if(pieces.begin(), pieces.end(), [](const p_piece &p){ return p->is_king(); });
        if (kit != pieces.end()) {
            pking = *kit;
        }
    }
    return pking;
}

position abstract_player::get_king_position()
{
    if (!pking) {
        auto kit = find_if(pieces.begin(), pieces.end(), [](const p_piece &p){ return p->is_king(); });
        if (kit == pieces.end()) {
            throw runtime_error("this player doesn't have a king");
        }
        pking = *kit;
    }

    return pking->get_position();
}

vector<position> abstract_player::get_all_available_target_positions(const board &bd) const
{
    vector<position> all_avail_pos;
    for (auto &&p : pieces) {
        p->update_moves(bd);
        auto& tp = p->get_avail_target_positions();
        all_avail_pos.insert(all_avail_pos.end(), tp.begin(), tp.end());
    }
    return all_avail_pos;
}

vector<pos_move> abstract_player::get_all_available_moves(const board &bd) const
{
    vector<pos_move> all_avail_moves;
    for (auto &&p : pieces) {
        p->update_moves(bd);
        if (p->is_movable()) {
            auto& from = p->get_position();
            auto& tp = p->get_avail_target_positions();
            for (auto &&m : tp) {
                all_avail_moves.emplace_back(from, m);
            }
        }
    }
    return all_avail_moves;
}
