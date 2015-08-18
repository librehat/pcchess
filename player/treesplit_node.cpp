#include "treesplit_node.h"
#include "random_player.h"
#include "../core/game.h"

using namespace std;
namespace mpi = boost::mpi;

treesplit_node::treesplit_node(const string &fen, const pos_move &mov, bool _my_turn, bool is_red_side, uint8_t noeat_half_rounds, node_ptr _parent, const int &_rank) :
    threaded_node(fen, mov, _my_turn, is_red_side, noeat_half_rounds, _parent),
    cn_rank(_rank),
    old_scores(0),
    new_visits(0),
    duplicated(false)
{}

treesplit_node::treesplit_node(const string &fen, bool is_red_side, uint8_t noeat_half_rounds, const int &_rank) :
    threaded_node(fen, is_red_side, noeat_half_rounds),
    cn_rank(_rank),
    old_scores(0),
    new_visits(0),
    duplicated(false)
{}

mpi::communicator treesplit_node::world_comm;
fast_ptr_hashtable<node, 1048576, true> treesplit_node::transtable;
thread_local queue<treesplit_node::msg_type> treesplit_node::output_queue;
const int treesplit_node::n_update = 5;
const int treesplit_node::n_duplicate = 25;

node::node_ptr treesplit_node::generate_root_node_with_move(const pos_move &m)
{
    random_player tr(true), tb(false);
    game updater_sim(&tr, &tb, no_eat_half_rounds);
    updater_sim.parse_fen(current_fen);
    updater_sim.move_piece(m);
    node_ptr child(new treesplit_node(updater_sim.get_fen(), m, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat(), nullptr, world_comm.rank()));
    return child;
}

void treesplit_node::expand(deque<pos_move> &hist, const int &score)
{
    visits++;
    new_visits++;
    scores += score;

    if (should_duplicate()) {
        output_queue.push(gen_duplicate_msg());
    } else if (should_update()) {
        output_queue.push(gen_update_msg());
    }

    if (depth - root_depth > max_depth) {
        return;
    }
    if (hist.empty()) {
        return;
    }

    pos_move next_move = hist.back();
    hist.pop_back();

    node_ptr child;
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
        size_t hash_val = hash_val_internal(fen, next_move, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat());
        int cn_id = hash_val % world_comm.size();//which compute node should contains this node
        if (cn_id == world_comm.rank()) {
            child = transtable[hash_val];
            if (child) {
                if (child->current_fen != fen || child->my_move != next_move) {
                    cerr << "collision: this hash_val (" << hash_val << ") and the hash_val in transtable (" << hash_value(*child) << ")" << endl;
                    throw std::runtime_error("collision");
                }
                child->parent = shared_from_this();
                child->backpropagate(child->scores, child->visits);
            } else {
                child = node_ptr(new treesplit_node(fen, next_move, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat(), shared_from_this(), cn_id));
                transtable.set(hash_val, child);
            }
        } else {
            child = node_ptr(new treesplit_node(fen, next_move, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat(), shared_from_this(), cn_id));
        }
        children_mutex.lock();
        children.push_back(child);
        children_mutex.unlock();
    } else {
        child = *child_iter;
        children_mutex.unlock();
    }

    child->expand(hist, score);
}

void treesplit_node::backpropagate(const int &score, const int &vis)
{
    if (auto _p = parent.lock()) {
        auto p = dynamic_pointer_cast<treesplit_node>(_p);
        if (p->parent.lock()) {//only if the parent is not root node
            if (p->should_duplicate()) {
                output_queue.push(p->gen_duplicate_msg());
            } else if (p->should_update()) {
                output_queue.push(p->gen_update_msg());
            }
        }
        p->visits += vis;
        p->new_visits += vis;
        p->scores += score;
        p->backpropagate(score, vis);
    }
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

bool treesplit_node::should_update() const
{
    static int rank = world_comm.rank();
    if (cn_rank != rank) {
        return new_visits.load(memory_order_acquire) > n_update;
    } else {
        return false;
    }
}

bool treesplit_node::should_duplicate() const
{
    static int rank = world_comm.rank();
    if (cn_rank == rank && !duplicated.load(memory_order_acquire)) {
        return visits.load(memory_order_relaxed) > n_duplicate;
    } else {
        return false;
    }
}

treesplit_node::msg_type treesplit_node::gen_update_msg()
{
    int sc = scores.load(memory_order_relaxed);
    int old_sc = old_scores.load(memory_order_relaxed);
    int delta_visits = new_visits.load(memory_order_relaxed);
    old_scores.store(sc, memory_order_relaxed);
    new_visits.store(0, memory_order_relaxed);
    return msg_type(cn_rank, sc - old_sc, current_fen, my_move, my_turn, red_side, no_eat_half_rounds, delta_visits);
}

treesplit_node::msg_type treesplit_node::gen_duplicate_msg()
{
    duplicated.store(true, memory_order_release);
    return msg_type(-1, scores.load(memory_order_relaxed), current_fen, my_move, my_turn, red_side, no_eat_half_rounds, visits.load(memory_order_relaxed));
}

void treesplit_node::remove_transmap_useless_entries()
{
    transtable.erase_unique_ptr();
}

void treesplit_node::handle_message(const msg_type &msg)
{
    size_t hash_val = hash_val_internal(get<2>(msg), get<3>(msg), get<4>(msg), get<5>(msg), get<6>(msg));
    int cn_id = hash_val % world_comm.size();//which compute node should contains this node
    node_ptr msg_node = node_ptr(new treesplit_node(get<2>(msg), get<3>(msg), get<4>(msg), get<5>(msg), get<6>(msg), nullptr, cn_id));
    msg_node->visits.store(get<7>(msg), memory_order_relaxed);
    msg_node->scores.store(get<1>(msg), memory_order_relaxed);
    dynamic_pointer_cast<treesplit_node>(msg_node)->old_scores.store(get<7>(msg), memory_order_relaxed);
    if (auto child = transtable[hash_val]) {
        if (get<0>(msg) == -1) {//a duplicate message, non-average mode since it's generated only once
            child->merge(*msg_node, false);
        } else {//an update message, merge it in non-average mode if it already exists
            child->merge(*msg_node, false);
        }
    } else {
        transtable.set(hash_val, msg_node);
    }
}
