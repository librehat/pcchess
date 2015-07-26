/*
 * This player is based on the Monte Carlo Tree Search algorithm
 * with UCT opimisation in search step
 */

#ifndef UCT_PLAYER_H
#define UCT_PLAYER_H

#include "../core/abstract_player.h"
#include "node.h"

class uct_player : public abstract_player
{
public:
    explicit uct_player(bool red = true);
    virtual ~uct_player();

    virtual bool think_next_move(pos_move &_move, const board &bd, unsigned int no_eat_half_rounds, const std::vector<pos_move> &banmoves);
    virtual void opponent_moved(const pos_move &m);
    virtual std::int64_t get_total_simulations() const;

    node *get_tree() const;

    static void text_archive_tree(std::ostream &os, node *);
    static void xml_archive_tree(std::ostream &os, node *);

protected:
    node *root;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(abstract_player);
        ar & BOOST_SERIALIZATION_NVP(root);
    }
};

#endif // UCT_PLAYER_H
