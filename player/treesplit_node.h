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

    explicit treesplit_node(const std::string &fen = std::string(), const pos_move &mov = pos_move(), bool _my_turn = true, bool is_red_side = true, std::uint8_t noeat_half_rounds = 0, node *_parent = nullptr);
    explicit treesplit_node(const std::string &fen, bool is_red_side, std::uint8_t noeat_half_rounds);

    node_ptr gen_child_with_a_move(const pos_move &m);

    void expand(std::deque<pos_move> &hist, const int &score);

    child_type get_best_child_msg();

    static std::size_t hash(const std::string &fen, const int &dep, const pos_move &m);
    static void clean_queue_map();//once the root is changed, call this function to "trim" the tree
    static void remove_transmap_invalid_entries();

private:
    friend class uct_treesplit_player;
    friend class boost::serialization::access;
    /*
     * msg_type includes
     * - target MPI rank
     * - history (last move included)
     * - score
     * - current FEN (of the node that send the msg)
     * - depth (of the node that send the msg)
     * - no_eat_half_rounds (of the node that send the msg)
     * - my_turn
     * - red_side
     * once received msg, the receiver needs to re-hash those jobs (find_child, hash, etc)
     */
    typedef std::tuple<int, std::deque<pos_move>, int, std::string, int, std::uint8_t, bool, bool> msg_type;

    static boost::mpi::communicator world_comm;
    static std::unordered_map<std::size_t, std::weak_ptr<node> > transmap;
    static std::mutex transmap_mutex;
    static thread_safe_queue<msg_type> output_queue;

    static void insert_node_from_msg(const msg_type &msg);

    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(threaded_node);
    }
};

#endif //TREESPLIT_NODE_H
