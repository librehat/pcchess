#ifndef THREADED_NODE_H
#define THREADED_NODE_H

#include "node.h"
#include <mutex>
#include <atomic>

class threaded_node : public node
{
public:
    explicit threaded_node(const std::string &fen = std::string(), bool _my_turn = true, bool is_red_side = true, std::int8_t noeat_half_rounds = 0, const std::vector<pos_move> &_banmoves = std::vector<pos_move>(), node *_parent = nullptr);

    bool select();
    void expand(std::deque<pos_move> &hist, const int &score);
    bool simulate();

    void backpropagate(const int &score);

    static std::int64_t get_total_simulations();

private:
    //they're instantiated per instance (each instance has its own visits, scores, children)
    std::mutex value_mutex;//protects visits, scores
    std::mutex children_mutex;//protects children

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(node);
    }

    static std::atomic<std::int64_t> total_simulations;
};

#endif //THREADED_NODE_H
