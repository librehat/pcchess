#ifndef TREESPLIT_NODE_H
#define TREESPLIT_NODE_H

#include "threaded_node.h"
#include <boost/mpi/communicator.hpp>
#include <unordered_map>
#include <tuple>
#include <queue>
#include <boost/serialization/deque.hpp>
#include "utils/serialization_tuple.hpp"
#include "utils/fast_ptr_hashtable.hpp"

class treesplit_node : public threaded_node
{
public:
    //next_move, visits, scores
    typedef std::tuple<pos_move, int, int> child_type;

    explicit treesplit_node(const std::string &fen = std::string(), const pos_move &mov = pos_move(), bool _my_turn = true, bool is_red_side = true, std::uint8_t noeat_half_rounds = 0, node_ptr _parent = node_ptr(), const int &_rank = 0);
    explicit treesplit_node(const std::string &fen, bool is_red_side, std::uint8_t noeat_half_rounds, const int &_rank);

    node_ptr generate_root_node_with_move(const pos_move &m);//generate a root node as if it was this node's child and update the move using m

    void expand(std::deque<pos_move> &hist, const int &score);

    void backpropagate(const int &score, const int &vis = 1);//the overloaded backpropagate will consider if the node belongs to other CN

    child_type get_best_child_msg();

    static void remove_transmap_useless_entries();
    static void clear_transtable() { transtable.clear(); }

private:
    const int cn_rank;//the CN's rank that this node belongs to

    friend class uct_treesplit_player;
    friend class boost::serialization::access;

    //rank, score, current_fen, my_move, my_turn, red_side, no_eat_half_rounds
    typedef std::tuple<int, int, std::string, pos_move, bool, bool, int> msg_type;

    static boost::mpi::communicator world_comm;
    static fast_ptr_hashtable<node, 262144> transtable;//2 ^ 18

    static thread_local std::queue<msg_type> output_queue;//thread_local so we don't need to use thread_safe_queue

    static void insert_node_from_msg(const msg_type &msg);

    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(threaded_node);
        ar & boost::serialization::make_nvp("cn_rank", const_cast<int &>(cn_rank));
    }
};

#endif //TREESPLIT_NODE_H
