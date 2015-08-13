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
    static std::size_t transtable_size() { return transtable.count(); }

private:
    /*
     * rank, score(s), current_fen, my_move, my_turn, red_side, no_eat_half_rounds, visists
     * rank == -1 means this message is a duplicate message
     */
    typedef std::tuple<int, int, std::string, pos_move, bool, bool, int, int> msg_type;

    const int cn_rank;//the CN's rank that this node belongs to
    std::atomic_int old_scores;//the scores when last update message was generated
    std::atomic_int new_visits;//after last time's update, how many visits it gets
    std::atomic_bool duplicated;//whether it's already duplicated on all other compute nodes

    bool should_update() const;
    bool should_duplicate() const;

    msg_type gen_update_msg();//generate an update message using this node's data, this function will update old_scores and new_visits
    msg_type gen_duplicate_msg();//genearte a duplicate message using this node's data, this function will set duplicated to true

    friend class uct_treesplit_player;
    friend class boost::serialization::access;

    static boost::mpi::communicator world_comm;
    static fast_ptr_hashtable<node, 1048576, true> transtable;//2 ^ 20

    static thread_local std::queue<msg_type> output_queue;//thread_local so we don't need to use thread_safe_queue

    static void handle_message(const msg_type &msg);

    static const int n_update;//after last update message, it has to be visisted more than n_update times before another message sent
    static const int n_duplicate;//similar, but this is for the node belongs to this compute node. once it gets visisted more than n_dup times, it'll be sent to all other compute nodes

    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(threaded_node);
        ar & boost::serialization::make_nvp("cn_rank", const_cast<int &>(cn_rank));
        ar & BOOST_SERIALIZATION_NVP(old_scores);
        ar & BOOST_SERIALIZATION_NVP(new_visits);
        ar & BOOST_SERIALIZATION_NVP(duplicated);
    }
};

#endif //TREESPLIT_NODE_H
