#ifndef NODE_H
#define NODE_H

#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/ptr_container/serialize_ptr_list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include "position.h"
#include "abstract_player.h"

class node : boost::noncopyable
{
public:
    //WARN: the node will take memory control of _our and _opp pointers!
    node(abstract_player* _our, abstract_player* _opp, bool _my_turn, node *_parent = nullptr);
    virtual ~node();

    int get_visits() const;
    int get_scores() const;
    double get_value() const;
    const pos_move& get_our_move() const;
    const pos_move& get_opp_move() const;
    double get_uct_val() const;

    void set_our_move(const pos_move &m);
    void set_opp_move(const pos_move &m);

    //three steps for MCTS
    virtual bool select();//return true if it did a successful simulation
    virtual void expand(std::list<pos_move> &our_hist, std::list<pos_move> &opp_hist, const int &score);
    virtual bool simulate();

    /*
     * select child according to the visit times
     * be aware of the return value could be nullptr (if children is empty)
     */
    boost::ptr_list<node>::iterator get_best_child();//best child which has highest visits
    boost::ptr_list<node>::iterator get_best_child_uct();//return best child which has maximum value of get_uct_val()
    boost::ptr_list<node>::iterator child_end();

    void backpropagate(const int &score);
    node* release_child(boost::ptr_list<node>::iterator i);

    /*
     * find the children with same moves (our_move or opp_move, up to the parental my_turn value).
     * return nullptr if no such child
     */
    boost::ptr_list<node>::iterator find_child(const pos_move &m);

    bool operator == (const node &b) const;
    bool operator != (const node &b) const;

    static int get_total_simulations();

protected:
    const bool my_turn;

    node* parent;
    //std::list<node *> children;
    boost::ptr_list<node> children;

    //"current": the state in this node
    abstract_player* our_curr;
    abstract_player* opp_curr;

    pos_move our_move;
    pos_move opp_move;

    int visits;
    int scores;//the sum of simulation result where win: +1 draw: 0 lose: -1

    static const int select_threshold;
    static const double uct_constant;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & my_turn;
        ar & parent;
        ar & BOOST_SERIALIZATION_NVP(children);
        ar & BOOST_SERIALIZATION_NVP(our_curr);
        ar & BOOST_SERIALIZATION_NVP(opp_curr);
        ar & BOOST_SERIALIZATION_NVP(our_move);
        ar & BOOST_SERIALIZATION_NVP(opp_move);
        ar & visits;
        ar & scores;
    }

    static int total_simulations;
};

typedef boost::ptr_list<node>::iterator node_iterator;

#endif // NODE_H
