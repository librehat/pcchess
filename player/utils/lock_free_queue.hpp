/*
 * this lock_free_queue template class mimic std::queue but provide thread-safe concurrently read and write
 * with some limitations (only one thread reads while another one writes)
 *
 * ABI is basically a mimic of std::queue (without swap() and back() function)
 */
#ifndef LOCK_FREE_QUEUE_HPP
#define LOCK_FREE_QUEUE_HPP

#include <array>
#include <atomic>

/*
 * the num is the length of underlying array
 * it can't be too small, otherwise new data won't be written because of lacking of available locations
 * if it's too big, it may use too much RAM
 */
template<class T, int num>
class lock_free_queue
{
public:
    lock_free_queue() :
        read_loc(0),
        write_loc(0)
    {}

    lock_free_queue(const lock_free_queue &q) :
        read_loc(q.read_loc.load()),
        write_loc(q.write_loc.load())
    {
        int uplim = write_loc.load();
        for (int i = read_loc.load(); i < uplim; ++i) {
            data[i % num] = q.data[i % num];
        }
    }

    bool empty() const { return read_loc.load() >= write_loc.load(); }

    /*
     * front() won't check location validity but assume this is not empty()
     */
    const T& front() const { return data[read_loc.load() % num]; }
    T& front() { return data[read_loc.load() % num]; }

    void push(const T &t) {
        if (write_loc - read_loc >= num) {
            throw std::range_error(std::string("underlying data is used up. can't write new data. read_loc: ") + std::to_string(read_loc.load()) + std::string("write_loc: ") + std::to_string(write_loc.load()));
        }
        data[write_loc.load() % num] = t;
        write_loc++;
    }

    void push(T &&t) {
        if (write_loc - read_loc >= num) {
            throw std::range_error(std::string("underlying data is used up. can't write new data. read_loc: ") + std::to_string(read_loc.load()) + std::string("write_loc: ") + std::to_string(write_loc.load()));
        }
        data[write_loc.load() % num] = t;
        write_loc++;
    }

    std::size_t size() const { return write_loc.load() - read_loc.load(); }

    void pop() { read_loc++; }

    void reset() {
        read_loc = 0;
        write_loc = 0;
        //data.fill(T());//don't really need to do this
    }

private:
    /*
     * thread safety is achieved by accesing the underlying array at different locations
     * it'll never write data into the location where it's being read
     */
    std::atomic_int read_loc;
    std::atomic_int write_loc;
    std::array<T, num> data;
};

#endif //LOCK_FREE_QUEUE_HPP
