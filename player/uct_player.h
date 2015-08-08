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

    bool think_next_move(pos_move &_move, const board &bd, std::uint8_t no_eat_half_rounds, const std::vector<pos_move> &banmoves);
    void opponent_moved(const pos_move &m);
    std::uint64_t get_total_simulations() const;

    inline node::node_ptr get_tree() const { return root; }

    static void text_archive_tree(std::ostream &os, node *);
    static void xml_archive_tree(std::ostream &os, node *);

protected:
    node::node_ptr root;
    std::atomic<uint64_t> selects;

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
