#ifndef THREADED_NODE_H
#define THREADED_NODE_H

#include "node.h"
#include <mutex>

class threaded_node : public node
{
public:
    explicit threaded_node(const std::string &fen = std::string(), const pos_move &mov = pos_move(), bool _my_turn = true, bool is_red_side = true, std::uint8_t noeat_half_rounds = 0, node *_parent = nullptr);
    explicit threaded_node(const std::string &fen, bool is_red_side, std::uint8_t noeat_half_rounds);

    bool select();
    void expand(std::deque<pos_move> &hist, const int &score);
    bool simulate();

protected:
    //they're instantiated per instance (each instance has its own visits, scores, children)
    std::mutex children_mutex;//protects children

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(node);
    }
};

#endif //THREADED_NODE_H
