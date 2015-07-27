#ifndef NODE_H
#define NODE_H

#include "../core/position.h"
#include "../core/abstract_player.h"
#include <boost/noncopyable.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/string.hpp>
#include <unordered_map>
#include <deque>
#include <cstdint>

class node : boost::noncopyable
{
public:
    typedef std::unordered_map<pos_move, node*>::iterator iterator;

    explicit node(const std::string &fen = std::string(), bool _my_turn = true, bool is_red_side = true, std::int8_t noeat_half_rounds = 0, node *_parent = nullptr);
    virtual ~node();

    node* make_shallow_copy() const;//used for root_uct_player
    node* make_shallow_copy_with_children() const;//used for slow_tree_uct_player
    node* gen_child_with_a_move(const pos_move &m);

    double get_value() const;
    double get_uct_val() const;

    //three steps for MCTS
    virtual bool select();//return true if it did a successful simulation
    virtual void expand(std::deque<pos_move> &hist, const int &score);
    virtual bool simulate();

    void merge(node &b, bool average_mode = false);//merge another node into this node. The target node needs to be in the same place in tree

    virtual void backpropagate(const int &score);
    int children_size() const;
    node* release_child(iterator i);
    inline void set_parent(node *n) { parent = n; }

    /*
     * select child according to the visit times
     * be aware of the return value could be nullptr (if children is empty)
     */
    iterator get_best_child();//best child which has highest visits
    iterator get_best_child_uct();//return best child which has maximum value of get_uct_val()
    iterator get_worst_child_uct();
    inline iterator find_child(const pos_move &m) { return children.find(m); }
    inline iterator child_end() { return children.end(); }

    bool is_same_place_in_tree(const node &b) const;
    bool is_basically_the_same(const node &b) const;//everything is the same except for those pointers such as parent

    bool operator == (const node &b) const;
    bool operator != (const node &b) const;

    static std::int64_t get_total_simulations();
    static void set_root_depth(const node * const r);//remember to call this function when you change the root node
    static void set_max_depth(const int &d);

protected:
    /*
     * if this is my turn to make a move, then the best move is the our_move of my best child.
     * if not, all my children's our_move are the same, but they have different opp_move
     */
    const bool my_turn;
    const bool red_side;

    node* parent;
    std::unordered_map<pos_move, node*> children;

    //use FEN string could save a LOT of data!
    std::string current_fen;
    int depth;//to get the _depth_, this needs to minus the root's depth
    int visits;
    int scores;//the sum of simulation result where win: +1 draw: 0 lose: -1
    std::int8_t no_eat_half_rounds;

    static int root_depth;
    static int max_depth;

    static const int select_threshold;
    static const double uct_constant;

    static bool compare_visits(const std::unordered_map<pos_move, node*>::value_type & x, const std::unordered_map<pos_move, node*>::value_type & y);
    static bool compare_uct(const std::unordered_map<pos_move, node*>::value_type & x, const std::unordered_map<pos_move, node*>::value_type & y);

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
        ar & boost::serialization::make_nvp("my_turn", const_cast<bool &>(my_turn));
        ar & boost::serialization::make_nvp("red_side", const_cast<bool &>(red_side));
        ar & BOOST_SERIALIZATION_NVP(parent);
        ar & BOOST_SERIALIZATION_NVP(children);
        ar & BOOST_SERIALIZATION_NVP(current_fen);
        ar & BOOST_SERIALIZATION_NVP(depth);
        ar & BOOST_SERIALIZATION_NVP(visits);
        ar & BOOST_SERIALIZATION_NVP(scores);
        ar & BOOST_SERIALIZATION_NVP(no_eat_half_rounds);
    }

    static std::int64_t total_simulations;
};

#endif // NODE_H
