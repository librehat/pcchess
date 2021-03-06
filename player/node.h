#ifndef NODE_H
#define NODE_H

#include "../core/position.h"
#include "../core/abstract_player.h"
#include <atomic>
#include <vector>
#include <deque>
#include <cstdint>
#include <boost/noncopyable.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include "utils/serialization_atomic.hpp"

class node : public std::enable_shared_from_this<node>, boost::noncopyable
{
public:
    typedef std::shared_ptr<node> node_ptr;
    typedef std::vector<node_ptr>::iterator iterator;

    explicit node(const std::string &fen = std::string(), const pos_move &_mov = pos_move(), bool _my_turn = true, bool is_red_side = true, std::uint8_t noeat_half_rounds = 0, node_ptr _parent = node_ptr());
    explicit node(const std::string &fen, bool is_red_side, std::uint8_t noeat_half_rounds);//a lazy constructor used to create the root node
    virtual ~node() {}

    node_ptr make_shallow_copy() const;//used for root_uct_player
    node_ptr make_shallow_copy_with_children() const;//used for slow_tree_uct_player
    node_ptr gen_child_with_a_move(const pos_move &m);

    double get_value() const;
    double get_uct_val() const;
    const pos_move &get_move() const { return my_move; }

    //three steps for MCTS
    virtual void select();//return true if it did a successful simulation
    virtual void expand(std::deque<pos_move> &hist, const int &score);
    void simulate();

    void merge(node &b, bool average_mode = false);//merge another node into this node. The target node needs to be in the same place in tree

    virtual void backpropagate(const int &score, const int &vis = 1);
    int children_size() const;
    node_ptr release_child(const iterator &i);
    inline void set_parent(const node_ptr &n) { parent = n; }
    inline void set_parent(const std::weak_ptr<node> &n) { parent = n; }

    /*
     * select child according to the visit times
     * be aware of the return value could be nullptr (if children is empty)
     */
    iterator get_best_child();//best child which has highest visits
    iterator get_best_child_uct();//return best child which has maximum value of get_uct_val()
    iterator get_worst_child_uct();
    iterator find_child(const pos_move &m);
    inline iterator child_end() { return children.end(); }

    bool is_same_place_in_tree(const node &b) const;
    bool is_basically_the_same(const node &b) const;//everything is the same except for those pointers such as parent

    bool operator == (const node &b) const;
    bool operator != (const node &b) const;

    static void set_root_depth(const node_ptr r);//remember to call this function when you change the root node
    static void set_max_depth(const int &d);

    friend std::size_t hash_value(const node &n) {
        return node::hash_val_internal(n.current_fen, n.my_move, n.my_turn, n.red_side, n.no_eat_half_rounds);
    }

protected:
    /*
     * if this is my turn to make a move, then the best move is the our_move of my best child.
     * if not, all my children's our_move are the same, but they have different opp_move
     */
    const bool my_turn;
    const bool red_side;

    std::weak_ptr<node> parent;
    std::vector<node_ptr> children;

    //use FEN string could save a LOT of data!
    const std::string current_fen;
    const pos_move my_move;
    std::atomic_int depth;//to get the _depth_, this needs to minus the root's depth
    std::atomic_int visits;
    std::atomic_int scores;//the sum of simulation result where win: +1 draw: 0 lose: -1
    std::atomic<std::uint8_t> no_eat_half_rounds;

    static std::atomic_int root_depth;
    static std::atomic_int max_depth;

    static const int select_threshold;
    static const double uct_constant;

    static bool compare_visits(const node_ptr& x, const node_ptr& y);
    static bool compare_uct(const node_ptr &x, const node_ptr &y);
    static std::size_t hash_val_internal(const std::string &fen, const pos_move &mov, const bool &myturn, const bool &red, const std::uint8_t &no_eat);

private:
    friend class treesplit_node;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
        ar & boost::serialization::make_nvp("my_turn", const_cast<bool &>(my_turn));
        ar & boost::serialization::make_nvp("red_side", const_cast<bool &>(red_side));
        ar & BOOST_SERIALIZATION_NVP(parent);
        ar & BOOST_SERIALIZATION_NVP(children);
        ar & boost::serialization::make_nvp("current_fen", const_cast<std::string &>(current_fen));
        ar & boost::serialization::make_nvp("my_move", const_cast<pos_move &>(my_move));
        ar & BOOST_SERIALIZATION_NVP(depth);
        ar & BOOST_SERIALIZATION_NVP(visits);
        ar & BOOST_SERIALIZATION_NVP(scores);
        ar & BOOST_SERIALIZATION_NVP(no_eat_half_rounds);
    }
};

namespace std
{
    template <>
    struct hash<node>
    {
        std::size_t operator()(const node& n) const
        {
            return hash_value(n);
        }
    };
}

#endif // NODE_H
