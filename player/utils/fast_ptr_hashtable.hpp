/*
 * A lock-free thread-safe hashtable for shared_ptr value
 *
 * inspired by http://preshing.com/20130605/the-worlds-simplest-lock-free-hash-table/
 *
 * some limitations:
 * - the key has to be std::uint64_t (hence you can use the return value of std::hash or boost::hash_value)
 * - the value has to be a std::shared_ptr template class type
 * - the hashtable's size is fixed and it has to be a power of 2
 * - there is no way to check if hashtable is full, so make a big enough table
 * - the complexities of count() and erase_unique_ptr() are linear to table's size
 *
 */

#ifndef FAST_PTR_HASHTABLE_HPP
#define FAST_PTR_HASHTABLE_HPP

#include <array>
#include <atomic>
#include <memory>
#include <algorithm>
#include <cstdint>

template <class T, int size>//size has to be a power of 2
class fast_ptr_hashtable
{
public:
    typedef std::shared_ptr<T> ptr_type;

    fast_ptr_hashtable() {}

    fast_ptr_hashtable(const fast_ptr_hashtable &) = delete;

    bool contains(const std::uint64_t &key) {
        for (std::uint64_t idx = hash_finalizer(key);; idx++) {
            idx &= size - 1;
            std::uint64_t probed_key = key_data[idx].load(std::memory_order_relaxed);
            if (probed_key == key)
                return true;
            if (probed_key == 0)
                return false;
        }
    }

    ptr_type at(const std::uint64_t &key) const {
        for (std::uint64_t idx = hash_finalizer(key);; idx++) {
            idx &= size - 1;
            std::uint64_t probed_key = key_data[idx].load(std::memory_order_relaxed);
            if (probed_key == key)
                return ptr_data[idx];
            if (probed_key == 0)
                return ptr_type();
        }
    }

    ptr_type operator[] (const std::uint64_t &key) const {
        return at(key);
    }

    void set(const std::uint64_t &key, const ptr_type &data) {
        assert(key != 0);
        assert(data);

        for (std::uint64_t idx = hash_finalizer(key);; idx++) {
            idx &= size - 1;

#ifndef NO_fast_ptr_hashtable_FAST_SET
            // Load the key that was there.
            std::uint64_t probed_key = key_data[idx].load(std::memory_order_relaxed);
            if (probed_key != key) {
                // The entry was either free, or contains another key.
                if (probed_key != 0)
                    continue;           // Usually, it contains another key. Keep probing.

                // The entry was free. Now let's try to take it using a CAS.
                std::uint64_t prev_key = 0;
                std::atomic_compare_exchange_strong_explicit(&(key_data[idx]), &prev_key, key, std::memory_order_relaxed, std::memory_order_relaxed);
                if ((prev_key != 0) && (prev_key != key))
                    continue;       // Another thread just stole it from underneath us.

            // Either we just added the key, or another thread did.
            }

            // Store the value in this array entry.
            std::atomic_store_explicit(&ptr_data[idx], data, std::memory_order_relaxed);
            return;
#else
            std::uint64_t prev_key = 0;
            std::atomic_compare_exchange_strong_explicit(&key_data[idx], &prev_key, key, std::memory_order_relaxed, std::memory_order_relaxed);
            if ((prev_key == 0) || (prev_key == key)) {
                std::atomic_store_explicit(&ptr_data[idx], data, std::memory_order_relaxed);
                return;
            }
#endif
        }
    }

    void erase(const std::uint64_t &key) {
        for (std::uint64_t idx = hash_finalizer(key);; idx++) {
            idx &= size - 1;
            std::uint64_t probed_key = key_data[idx].load(std::memory_order_relaxed);
            if (probed_key == key) {
                key_data[idx].store(0);
                std::atomic_store(&ptr_data[idx], ptr_type());
                return;
            }
            if (probed_key == 0)//it doesn't exist
                return;
        }
    }

    void erase_unique_ptr() {
#pragma omp parallel for
        for (std::uint64_t i = 0; i < size; i++) {
            if (key_data[i].load(std::memory_order_relaxed) != 0) {
                if (auto p = std::atomic_load_explicit(&ptr_data[i], std::memory_order_relaxed)) {
                    if (p.unique()) {
                        key_data[i].store(0);
                        std::atomic_store(&ptr_data[i], ptr_type());
                    }
                }
            }
        }
    }

    std::size_t count() const {
        std::atomic_size_t count;
#pragma omp parallel for
        for (std::uint64_t idx = 0; idx < size; idx++) {
            if ((key_data[idx].load(std::memory_order_relaxed) != 0)
             && (std::atomic_load_explicit(&ptr_data[idx], std::memory_order_relaxed))) {
                count++;
            }
        }
        return count;
    }

    std::uint64_t max_size() const { return size; }

private:
    std::array<ptr_type, size> ptr_data;
    std::array<std::atomic<std::uint64_t>, size> key_data;

    //64-bit finalizer from http://code.google.com/p/smhasher/wiki/MurmurHash3
    inline static std::uint64_t hash_finalizer(std::uint64_t h) {
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccd;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53;
        h ^= h >> 33;
        return h;
    }
};

#endif //FAST_PTR_HASHTABLE_HPP
