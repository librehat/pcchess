#ifndef TREESPLIT_NODE_H
#define TREESPLIT_NODE_H

#include "threaded_node.h"
#include <boost/mpi/communicator.hpp>
#include <unordered_map>
#include <tuple>
#include <boost/serialization/deque.hpp>
#include "utils/serialization_tuple.hpp"
#include "utils/thread_safe_queue.hpp"

class treesplit_node : public threaded_node
{
public:
    //next_move, visits, scores
    typedef std::tuple<pos_move, int, int> child_type;

    explicit treesplit_node(const std::string &fen = std::string(), const pos_move &mov = pos_move(), bool _my_turn = true, bool is_red_side = true, std::uint8_t noeat_half_rounds = 0, node_ptr _parent = node_ptr());
    explicit treesplit_node(const std::string &fen, bool is_red_side, std::uint8_t noeat_half_rounds);
    explicit treesplit_node(const treesplit_node &b);

    node_ptr gen_child_with_a_move(const pos_move &m);

    void expand(std::deque<pos_move> &hist, const int &score);

    child_type get_best_child_msg();

    static void remove_transmap_useless_entries();
    static thread_local void clear_oq() { std::queue<msg_type>().swap(output_queue); }

private:
    friend class uct_treesplit_player;
    friend class boost::serialization::access;

    //rank, score, current_fen my_move, my_turn, red_side, no_eat_half_rounds
    typedef std::tuple<int, int, std::string, pos_move, bool, bool, int> msg_type;

    static boost::mpi::communicator world_comm;
    static std::unordered_map<std::size_t, node_ptr> transmap;
    static std::unordered_map<std::size_t, node_ptr> remote_cache;
    static std::mutex transmap_mutex;
    static std::mutex remote_cache_mutex;
    static thread_local std::queue<msg_type> output_queue;//thread_local so we don't need to use thread_safe_queue

    static void insert_node_from_msg(const msg_type &msg);

    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(threaded_node);
    }
};

#endif //TREESPLIT_NODE_H
