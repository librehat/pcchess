#ifndef NODE_H
#define NODE_H

#include <boost/utility.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/unordered_map.hpp>
#include "../core/position.h"
#include "../core/abstract_player.h"
#include <deque>
#include <cstdint>
#include <unordered_map>

class node : boost::noncopyable
{
public:
    typedef std::unordered_map<pos_move, node*>::iterator node_iterator;

    //WARN: the node will take memory control of _our and _opp pointers!
    explicit node(abstract_player* _our = nullptr, abstract_player* _opp = nullptr, bool _my_turn = true, unsigned int noeat_half_rounds = 0, const std::vector<pos_move> &_banmoves = std::vector<pos_move>(), node *_parent = nullptr);
    virtual ~node();

    double get_value() const;
    double get_uct_val() const;

    //three steps for MCTS
    virtual bool select();//return true if it did a successful simulation
    virtual void expand(std::deque<pos_move> &hist, const int &score);
    virtual bool simulate();

    void merge(node &b);//merge another node into this node. The target node needs to be in the same place in tree

    virtual void backpropagate(const int &score);
    int children_size() const;
    node* release_child(node_iterator i);

    /*
     * select child according to the visit times
     * be aware of the return value could be nullptr (if children is empty)
     */
    node_iterator get_best_child();//best child which has highest visits
    node_iterator get_best_child_uct();//return best child which has maximum value of get_uct_val()
    node_iterator get_worst_child_uct();
    inline node_iterator find_child(const pos_move &m) { return children.find(m); }
    inline node_iterator child_end() { return children.end(); }

    bool is_same_place_in_tree(const node &b) const;
    bool is_basically_the_same(const node &b) const;//everything is the same except for those pointers such as parent

    bool operator == (const node &b) const;
    bool operator != (const node &b) const;

    static std::int64_t get_total_simulations();

protected:
    /*
     * if this is my turn to make a move, then the best move is the our_move of my best child.
     * if not, all my children's our_move are the same, but they have different opp_move
     */
    const bool my_turn;

    node* parent;
    std::unordered_map<pos_move, node*> children;

    //"current": the state in this node
    abstract_player* our_curr;
    abstract_player* opp_curr;

    int depth;
    int visits;
    int scores;//the sum of simulation result where win: +1 draw: 0 lose: -1

    unsigned int no_eat_half_rounds;
    std::vector<pos_move> banmoves;//banmoves is always _our_ banmoves

    static const int select_threshold;
    static const double uct_constant;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
        ar & boost::serialization::make_nvp("my_turn", const_cast<bool &>(my_turn));
        ar & BOOST_SERIALIZATION_NVP(parent);
        ar & BOOST_SERIALIZATION_NVP(children);
        ar & BOOST_SERIALIZATION_NVP(our_curr);
        ar & BOOST_SERIALIZATION_NVP(opp_curr);
        ar & BOOST_SERIALIZATION_NVP(depth);
        ar & BOOST_SERIALIZATION_NVP(visits);
        ar & BOOST_SERIALIZATION_NVP(scores);
        ar & BOOST_SERIALIZATION_NVP(no_eat_half_rounds);
        ar & BOOST_SERIALIZATION_NVP(banmoves);
    }

    static std::int64_t total_simulations;
};

#endif // NODE_H
