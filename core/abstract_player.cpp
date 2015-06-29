#include "abstract_player.h"
#include "soldier.h"
#include "cannon.h"
#include "rook.h"
#include "horse.h"
#include "elephant.h"
#include "advisor.h"
#include "general.h"
#include <stdexcept>
#include <memory>

using namespace std;

abstract_player::abstract_player(board &bd) :
    checked(false),
    checkmated(false),
    m_board(bd)
{}

abstract_player::abstract_player(const abstract_player &b, board &new_board) :
    pieces(b.pieces),
    checked(b.checked),
    checkmated(b.checkmated),
    m_board(new_board)
{}

abstract_player::~abstract_player()
{}

void abstract_player::add(p_piece p)
{
    pieces.push_back(p);
}

void abstract_player::remove(p_piece p)
{
    if (p->abbr_name() == 'G') {
        checkmated = true;
    }
    pieces.remove(p);
}

void abstract_player::init_pieces(bool opposite)
{
    if (!pieces.empty()) {
        throw runtime_error("Error. pieces list is not empty when init_pieces is called.");
    }

    p_piece p;

    //soldiers (pawns)
    p = p_piece(new soldier(0, opposite ? 3 : 6, opposite, m_board));
    pieces.push_back(p);
    p = p_piece(new soldier(2, opposite ? 3 : 6, opposite, m_board));
    pieces.push_back(p);
    p = p_piece(new soldier(4, opposite ? 3 : 6, opposite, m_board));
    pieces.push_back(p);
    p = p_piece(new soldier(6, opposite ? 3 : 6, opposite, m_board));
    pieces.push_back(p);
    p = p_piece(new soldier(8, opposite ? 3 : 6, opposite, m_board));
    pieces.push_back(p);

    //cannons
    p = p_piece(new cannon(1, opposite ? 2 : 7, opposite, m_board));
    pieces.push_back(p);
    p = p_piece(new cannon(7, opposite ? 2 : 7, opposite, m_board));
    pieces.push_back(p);

    //rooks
    p = p_piece(new rook(0, opposite ? 0 : 9, opposite, m_board));
    pieces.push_back(p);
    p = p_piece(new rook(8, opposite ? 0 : 9, opposite, m_board));
    pieces.push_back(p);

    //horses
    p = p_piece(new horse(1, opposite ? 0 : 9, opposite, m_board));
    pieces.push_back(p);
    p = p_piece(new horse(7, opposite ? 0 : 9, opposite, m_board));
    pieces.push_back(p);

    //elephants
    p = p_piece(new elephant(2, opposite ? 0 : 9, opposite, m_board));
    pieces.push_back(p);
    p = p_piece(new elephant(6, opposite ? 0 : 9, opposite, m_board));
    pieces.push_back(p);

    //advisors
    p = p_piece(new advisor(3, opposite ? 0 : 9, opposite, m_board));
    pieces.push_back(p);
    p = p_piece(new advisor(5, opposite ? 0 : 9, opposite, m_board));
    pieces.push_back(p);

    //general
    p = p_piece(new general(4, opposite ? 0 : 9, opposite, m_board));
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

bool abstract_player::think_next_move(pos_move &m)
{
    return think_next_move(m[0], m[1]);
}
