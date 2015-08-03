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

treesplit_node::treesplit_node(const treesplit_node &b) :
    threaded_node(b)
{}

mpi::communicator treesplit_node::world_comm;
unordered_map<size_t, node::node_ptr> treesplit_node::transmap;
mutex treesplit_node::transmap_mutex;
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
        size_t hash_val = hash_val_internal(fen, !my_turn, red_side);
        int cn_id = hash_val % world_comm.size();//which compute node should contains this node
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
                children_mutex.lock();
                children.push_back(child);
                children_mutex.unlock();
            }
        } else {//the statistics needs to be sent to other node
            treesplit_node msg_node(fen, next_move, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat());
            msg_type msg(cn_id, score, msg_node);
            output_queue.push(msg);
            msg_node.expand(hist, score);
            return;
        }
    } else {
        child = *child_iter;
        children_mutex.unlock();
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
        if(it->second.unique()) {//FIXME: they may not be "useless"
            it = transmap.erase(it);
        } else {
            ++it;
        }
    }
    transmap_mutex.unlock();
}

void treesplit_node::insert_node_from_msg(const msg_type &msg)
{
#ifdef _DEBUG
    cout <<  "[" << world_comm.rank() << "] insert_node_from_msg" << endl;
#endif

    const treesplit_node &msg_node = get<2>(msg);
    size_t hash_val = hash_value(dynamic_cast<const node &>(msg_node));
    node_ptr child;
    transmap_mutex.lock();
    auto ctm = transmap.find(hash_val);
    if (ctm != transmap.end()) {
        child = ctm->second;
        transmap_mutex.unlock();
        child->backpropagate(get<1>(msg));
    } else {
        child = node_ptr(new treesplit_node(msg_node.current_fen, msg_node.my_move, msg_node.my_turn, msg_node.red_side, msg_node.no_eat_half_rounds));
        transmap.emplace(hash_val, child);
        transmap_mutex.unlock();
    }
}
