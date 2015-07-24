#ifndef THREADED_NODE_H
#define THREADED_NODE_H

#include "node.h"
#include <mutex>
#include <atomic>

class threaded_node : public node
{
public:
    explicit threaded_node(abstract_player* _our = nullptr, abstract_player* _opp = nullptr, bool _my_turn = true, node *_parent = nullptr);

    bool select();
    void expand(std::deque<pos_move> &hist, const int &score);
    bool simulate();

    static int get_total_simulations();

private:
    //they're instantiated per instance (each instance has its own visits, scores, children)
    std::mutex value_mutex;//protects visits, scores
    std::mutex children_mutex;//protects children

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(node);
    }

    static std::atomic<int> total_simulations;
};

#endif //THREADED_NODE_H