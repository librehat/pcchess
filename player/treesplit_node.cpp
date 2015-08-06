#include "treesplit_node.h"
#include "random_player.h"
#include "../core/game.h"

using namespace std;
namespace mpi = boost::mpi;

treesplit_node::treesplit_node(const string &fen, const pos_move &mov, bool _my_turn, bool is_red_side, uint8_t noeat_half_rounds, node_ptr _parent, const int &_rank) :
    threaded_node(fen, mov, _my_turn, is_red_side, noeat_half_rounds, _parent),
    cn_rank(_rank)
{}

treesplit_node::treesplit_node(const string &fen, bool is_red_side, uint8_t noeat_half_rounds, const int &_rank) :
    threaded_node(fen, is_red_side, noeat_half_rounds),
    cn_rank(_rank)
{}

mpi::communicator treesplit_node::world_comm;
fast_ptr_hashtable<node, 1048576> treesplit_node::transtable;
thread_local queue<treesplit_node::msg_type> treesplit_node::output_queue;

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
            if (child = transtable[hash_val]) {
                if (child->current_fen != fen || child->my_move.load() != next_move) {
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

    if (dynamic_pointer_cast<treesplit_node>(child)->cn_rank != world_comm.rank()) {
        msg_type msg(dynamic_pointer_cast<treesplit_node>(child)->cn_rank, score, child->current_fen, next_move, child->my_turn, red_side, child->no_eat_half_rounds);
        output_queue.push(msg);
    }

    child->expand(hist, score);
}

void treesplit_node::backpropagate(const int &score, const int &vis)
{
    if (auto p = parent.lock()) {
        if (dynamic_pointer_cast<treesplit_node>(p)->cn_rank != world_comm.rank()) {
            msg_type msg(dynamic_pointer_cast<treesplit_node>(p)->cn_rank, score, p->current_fen, p->my_move, p->my_turn, p->red_side, p->no_eat_half_rounds);
            output_queue.push(msg);
        }
        p->visits += vis;
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

void treesplit_node::remove_transmap_useless_entries()
{
    transtable.erase_unique_ptr();
}

void treesplit_node::insert_node_from_msg(const msg_type &msg)
{
    size_t hash_val = hash_val_internal(get<2>(msg), get<3>(msg), get<4>(msg), get<5>(msg), get<6>(msg));
    node_ptr child;
    if (child = transtable[hash_val]) {
        child->backpropagate(get<1>(msg));
    } else {
        child = node_ptr(new treesplit_node(get<2>(msg), get<3>(msg), get<4>(msg), get<5>(msg), get<6>(msg), nullptr, get<0>(msg)));
        transtable.set(hash_val, child);
    }
    child->scores += get<1>(msg);
    child->visits += 1;
}
