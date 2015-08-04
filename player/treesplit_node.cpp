#include "treesplit_node.h"
#include "random_player.h"
#include "../core/game.h"

using namespace std;
namespace mpi = boost::mpi;

treesplit_node::treesplit_node(const string &fen, const pos_move &mov, bool _my_turn, bool is_red_side, uint8_t noeat_half_rounds, node_ptr _parent) :
    threaded_node(fen, mov, _my_turn, is_red_side, noeat_half_rounds, _parent)
{}

treesplit_node::treesplit_node(const string &fen, bool is_red_side, uint8_t noeat_half_rounds) :
    threaded_node(fen, is_red_side, noeat_half_rounds)
{}

/*treesplit_node::treesplit_node(const treesplit_node &b) :
    threaded_node(b)
{}*/

mpi::communicator treesplit_node::world_comm;
unordered_map<size_t, node::node_ptr> treesplit_node::transmap;
unordered_map<size_t, node::node_ptr> treesplit_node::remote_cache;
mutex treesplit_node::transmap_mutex;
mutex treesplit_node::remote_cache_mutex;
thread_local queue<treesplit_node::msg_type> treesplit_node::output_queue;

node::node_ptr treesplit_node::gen_child_with_a_move(const pos_move &m)
{
    random_player tr(true), tb(false);
    game updater_sim(&tr, &tb, no_eat_half_rounds);
    updater_sim.parse_fen(current_fen);
    updater_sim.move_piece(m);
    node_ptr child(new treesplit_node(updater_sim.get_fen(), m, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat(), shared_from_this()));
    return child;
}

void treesplit_node::expand(deque<pos_move> &hist, const int &score)
{
    visits++;
    scores += score;

    if (depth - root_depth > max_depth) {
        return;
    }
    if (hist.empty()) {
        return;
    }

    pos_move next_move = hist.back();
    hist.pop_back();

    node_ptr child;
    size_t hash_val;
    int cn_id;
    children_mutex.lock();
    auto child_iter = find_child(next_move);
    if (child_iter == children.end()) {
        children_mutex.unlock();
        random_player tr(true), tb(false);
        game updater_sim(&tr, &tb, no_eat_half_rounds);
        updater_sim.parse_fen(current_fen);
        updater_sim.move_piece(next_move);
        if (my_turn && updater_sim.is_player_in_check(red_side)) {
            return;//we're checked! don't make this move
        }

        string fen = updater_sim.get_fen();
        hash_val = hash_val_internal(fen, next_move, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat());
        cn_id = hash_val % world_comm.size();//which compute node should contains this node
        if (cn_id == world_comm.rank()) {
            transmap_mutex.lock();
            auto ctm = transmap.find(hash_val);
            if (ctm != transmap.end()) {
                child = ctm->second;
                transmap_mutex.unlock();
                child->parent = shared_from_this();
                child->backpropagate(child->scores, child->visits);
            } else {
                child = node_ptr(new treesplit_node(fen, next_move, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat(), shared_from_this()));
                transmap.emplace(hash_val, child);
                transmap_mutex.unlock();
            }
        } else {
            child = node_ptr(new treesplit_node(fen, next_move, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat(), shared_from_this()));
            remote_cache_mutex.lock();
            remote_cache.emplace(hash_val, child);
            remote_cache_mutex.unlock();
        }
        children_mutex.lock();
        children.push_back(child);
        children_mutex.unlock();
    } else {
        child = *child_iter;
        children_mutex.unlock();
        hash_val = hash_value(*dynamic_pointer_cast<node>(child));
        cn_id = hash_val % world_comm.size();
    }

    if (cn_id != world_comm.rank()) {
        msg_type msg(cn_id, score, child->current_fen, next_move, child->my_turn, red_side, child->no_eat_half_rounds);
        output_queue.push(msg);
    }

    child->expand(hist, score);
}

treesplit_node::child_type treesplit_node::get_best_child_msg()
{
    auto child_it = get_best_child();
    if (child_it != children.end()) {
        node::node_ptr child = *child_it;
        return child_type(child->my_move, child->visits, child->scores);
    } else {
        return child_type();
    }
}

void treesplit_node::remove_transmap_useless_entries()
{
    transmap_mutex.lock();
    for (auto it = transmap.begin(); it != transmap.end();) {
        if(it->second.unique()) {
            it = transmap.erase(it);
        } else {
            ++it;
        }
    }
    transmap_mutex.unlock();
    remote_cache_mutex.lock();
    for (auto it = remote_cache.begin(); it != remote_cache.end();) {
        if(it->second.unique()) {
            it = remote_cache.erase(it);
        } else {
            ++it;
        }
    }
    remote_cache_mutex.unlock();
}

void treesplit_node::insert_node_from_msg(const msg_type &msg)
{
    size_t hash_val = hash_val_internal(get<2>(msg), get<3>(msg), get<4>(msg), get<5>(msg), get<6>(msg));
    node_ptr child;
    transmap_mutex.lock();
    auto ctm = transmap.find(hash_val);
    if (ctm != transmap.end()) {
        child = ctm->second;
        transmap_mutex.unlock();
        child->backpropagate(get<1>(msg));
    } else {
        child = node_ptr(new treesplit_node(get<2>(msg), get<3>(msg), get<4>(msg), get<5>(msg), get<6>(msg)));
        transmap.emplace(hash_val, child);
        transmap_mutex.unlock();
    }
}
