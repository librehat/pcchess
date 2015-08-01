#include "treesplit_node.h"
#include "random_player.h"
#include "../core/game.h"

using namespace std;
namespace mpi = boost::mpi;

treesplit_node::treesplit_node(const string &fen, const pos_move &mov, bool _my_turn, bool is_red_side, uint8_t noeat_half_rounds, node *_parent) :
    threaded_node(fen, mov, _my_turn, is_red_side, noeat_half_rounds, _parent)
{}

treesplit_node::treesplit_node(const string &fen, bool is_red_side, uint8_t noeat_half_rounds) :
    threaded_node(fen, is_red_side, noeat_half_rounds)
{}

mpi::communicator treesplit_node::world_comm;
unordered_map<size_t, weak_ptr<node> > treesplit_node::transmap;
mutex treesplit_node::transmap_mutex;
queue<treesplit_node::msg_type> treesplit_node::output_queue;
mutex treesplit_node::oq_mutex;
queue<treesplit_node::msg_type> treesplit_node::input_queue;
mutex treesplit_node::iq_mutex;

node::node_ptr treesplit_node::gen_child_with_a_move(const pos_move &m)
{
    random_player tr(true), tb(false);
    game updater_sim(&tr, &tb, no_eat_half_rounds);
    updater_sim.parse_fen(current_fen);
    updater_sim.move_piece(m);
    node_ptr child(new treesplit_node(updater_sim.get_fen(), m, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat(), this));
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
        if (my_turn) {
            if (updater_sim.is_player_in_check(red_side)) {
                return;//we're checked! don't make this move
            }
        }

        string fen = updater_sim.get_fen();
        size_t hash_val = hash(fen, depth + 1, next_move);
        int cn_id = hash_val % world_comm.size();//which compute node should contains this node
        if (cn_id == world_comm.rank()) {
            transmap_mutex.lock();
            auto ctm = transmap.find(hash_val);
            if (ctm != transmap.end()) {
                child = node_ptr(ctm->second);
                transmap_mutex.unlock();
                child->parent = this;
                child->backpropagate(child->scores, child->visits);
            } else {
                child = node_ptr(new treesplit_node(fen, next_move, !my_turn, red_side, updater_sim.get_half_rounds_since_last_eat(), this));
                transmap.emplace(hash_val, weak_ptr<node>(child));
                transmap_mutex.unlock();
            }
            children_mutex.lock();
            children.push_back(child);
        } else {//the statistics nees to be sent to other node
            hist.push_back(next_move);
            msg_type msg(cn_id, hist, score, current_fen, depth, no_eat_half_rounds, my_turn, red_side);
            oq_mutex.lock();
            output_queue.push(msg);
            oq_mutex.unlock();
            return;
        }
    } else {
        child = *child_iter;
    }
    children_mutex.unlock();
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

size_t treesplit_node::hash(const string &fen, const int &dep, const pos_move &m)
{
    std::size_t seed;
    boost::hash_combine(seed, fen);
    boost::hash_combine(seed, dep);
    boost::hash_combine(seed, m);
    return seed;
}

void treesplit_node::clean_queue_map()
{
    remove_transmap_invalid_entries();
    iq_mutex.lock();
    queue<treesplit_node::msg_type>().swap(input_queue);
    iq_mutex.unlock();
    oq_mutex.lock();
    queue<treesplit_node::msg_type>().swap(output_queue);
    oq_mutex.unlock();
}

void treesplit_node::remove_transmap_invalid_entries()
{
    transmap_mutex.lock();
    for (auto it = transmap.begin(); it != transmap.end();) {
        if(it->second.expired()) {
            it = transmap.erase(it);
        } else {
            ++it;
        }
    }
    transmap_mutex.unlock();
}

void treesplit_node::insert_node_from_msg(const msg_type &msg)
{
    deque<pos_move> hist = get<1>(msg);
    random_player tr(true), tb(false);
    game updater_sim(&tr, &tb, get<5>(msg));
    updater_sim.parse_fen(get<3>(msg));
    updater_sim.move_piece(hist.back());
    string fen = updater_sim.get_fen();
    size_t hash_val = hash(fen, get<4>(msg) + 1, hist.back());
    node_ptr child;
    transmap_mutex.lock();
    auto ctm = transmap.find(hash_val);
    if (ctm != transmap.end()) {
        child = node_ptr(ctm->second);
        transmap_mutex.unlock();
        child->backpropagate(get<2>(msg));
    } else {
        child = node_ptr(new treesplit_node(fen, hist.back(), !get<6>(msg), get<7>(msg), updater_sim.get_half_rounds_since_last_eat()));
        transmap.emplace(hash_val, weak_ptr<node>(child));
        transmap_mutex.unlock();
    }
    hist.pop_back();
    child->expand(hist, get<2>(msg));
}
