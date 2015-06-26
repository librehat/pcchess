#include "abstract_player.h"
#include "soldier.h"
#include "cannon.h"
#include "rook.h"
#include "horse.h"
#include "elephant.h"
#include "advisor.h"
#include "general.h"
#include <stdexcept>

using namespace std;

abstract_player::abstract_player(board &bd) :
    m_board(bd),
    checked(false),
    checkmated(false)
{
}

abstract_player::~abstract_player()
{
    for (auto it = pieces.begin(); it != pieces.end(); ++it) {
        delete *it;
    }
}

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
    delete p;
}

void abstract_player::init_pieces(bool opposite)
{
    if (!pieces.empty()) {
        throw runtime_error("Error. pieces list is not empty when init_pieces is called.");
    }

    p_piece p;

    //soldiers (pawns)
    p = new soldier(0, opposite ? 3 : 6, opposite, m_board);
    pieces.push_back(p);
    p = new soldier(2, opposite ? 3 : 6, opposite, m_board);
    pieces.push_back(p);
    p = new soldier(4, opposite ? 3 : 6, opposite, m_board);
    pieces.push_back(p);
    p = new soldier(6, opposite ? 3 : 6, opposite, m_board);
    pieces.push_back(p);
    p = new soldier(8, opposite ? 3 : 6, opposite, m_board);
    pieces.push_back(p);

    //cannons
    p = new cannon(1, opposite ? 2 : 7, opposite, m_board);
    pieces.push_back(p);
    p = new cannon(7, opposite ? 2 : 7, opposite, m_board);
    pieces.push_back(p);

    //rooks
    p = new rook(0, opposite ? 0 : 9, opposite, m_board);
    pieces.push_back(p);
    p = new rook(8, opposite ? 0 : 9, opposite, m_board);
    pieces.push_back(p);

    //horses
    p = new horse(1, opposite ? 0 : 9, opposite, m_board);
    pieces.push_back(p);
    p = new horse(7, opposite ? 0 : 9, opposite, m_board);
    pieces.push_back(p);

    //elephants
    p = new elephant(2, opposite ? 0 : 9, opposite, m_board);
    pieces.push_back(p);
    p = new elephant(6, opposite ? 0 : 9, opposite, m_board);
    pieces.push_back(p);

    //advisors
    p = new advisor(3, opposite ? 0 : 9, opposite, m_board);
    pieces.push_back(p);
    p = new advisor(5, opposite ? 0 : 9, opposite, m_board);
    pieces.push_back(p);

    //general
    p = new general(4, opposite ? 0 : 9, opposite, m_board);
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
