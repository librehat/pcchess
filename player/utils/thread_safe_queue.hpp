#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <queue>
#include <mutex>

template <class T>
class thread_safe_queue
{
public:
    thread_safe_queue() = default;

    thread_safe_queue(const thread_safe_queue &other) = delete;

    thread_safe_queue(thread_safe_queue &&other) {
        std::unique_lock<std::mutex> l1(queue_mutex, std::defer_lock), l2(other.queue_mutex, std::defer_lock);
        std::lock(l1, l2);
        queue_data = std::move(other.queue_data);
    }

    void push(const T &x) {
        std::lock_guard<std::mutex> lg(queue_mutex);
        queue_data.push(x);
    }

    void push(T &&x) {
        std::lock_guard<std::mutex> lg(queue_mutex);
        queue_data.push(x);
    }

    void pop() {
        std::lock_guard<std::mutex> lg(queue_mutex);
        queue_data.pop();
    }

    bool empty() {
        std::lock_guard<std::mutex> lg(queue_mutex);
        return queue_data.empty();
    }

    T& front() {
        std::lock_guard<std::mutex> lg(queue_mutex);
        return queue_data.front();
    }

    T& back() {
        std::lock_guard<std::mutex> lg(queue_mutex);
        return queue_data.back();
    }

    std::size_t size() {
        std::lock_guard<std::mutex> lg(queue_mutex);
        return queue_data.size();
    }

    void swap(thread_safe_queue &other) {
        std::unique_lock<std::mutex> l1(queue_mutex, std::defer_lock), l2(other.queue_mutex, std::defer_lock);
        std::lock(l1, l2);
        std::swap(queue_data, other.queue_data);
    }

    void reset() {
        std::queue<T>().swap(queue_data);
    }

private:
    std::queue<T> queue_data;
    std::mutex queue_mutex;
};

#endif //THREAD_SAFE_QUEUE_HPP
