/*
 * This player is based on the Monte Carlo Tree Search algorithm
 * with UCT opimisation in search step
 */

#ifndef UCT_PLAYER_H
#define UCT_PLAYER_H

#include "abstract_player.h"
#include "node.h"

class uct_player : public abstract_player
{
public:
    uct_player(const abstract_player* const _opp = nullptr, bool opposite = false);
    virtual ~uct_player();

    virtual bool think_next_move(pos_move &_move, const board &);
    virtual void opponent_moved(const pos_move &m);
    virtual int get_total_simulations() const;

    node *get_tree() const;
    void text_archive_tree(std::ostream &os) const;

protected:
    node *root;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<abstract_player>(*this);
        ar & root;
    }
};

#endif // UCT_PLAYER_H
