#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#include "algo/list/range.hpp"
#include "algo/search_map/h.hpp"

#pragma once

template <typename val_t>

struct HMapStatic_HNode {
    Range<const char> key;
    val_t value;
};

template <std::size_t cap, typename val_t>

// dumb hashmap
class HMapStatic : public HMap<val_t> {
  private:
    using HNode = HMapStatic_HNode<val_t>;

    inline bool key_eq(Range<const char> l, Range<const char> r) const {
        if (l.p_end - l.p_start != r.p_end - r.p_start) {
            return false;
        }

        while (r.p_start < r.p_end) {
            if (*(l.p_start++) != *(r.p_start++)) {
                return false;
            }
        }

        return true;
    }

    inline void node_init(HNode *node, Range<const char> key, val_t value) {
        std::size_t size = key.size();
        char *cpy_start = new char[size];

        std::memcpy(cpy_start, key.p_start, size);

        node->key.p_start = cpy_start;
        node->key.p_end = cpy_start + size;
        node->value = value;
    }

  public:
    std::array<Range<HNode>, cap> bucket_list {};

    // very dumb hash function
    constexpr std::size_t hash_let(Range<const char> key) const {
        std::uint64_t hash = 0;

        for (const char *key_now = key.p_start; key_now < key.p_end; ++key_now) {
            hash = hash * 31 + *key_now;
        }

        return hash;
    }

    constexpr std::optional<val_t> find(Range<const char> key) const {
        std::size_t hash = this->hash_let(key);
        const Range<HNode> *bucket = this->bucket_list.data() + (hash % cap);

        if (bucket->p_start == nullptr) {
            return std::nullopt;
        }

        for (HNode *bucket_now = bucket->p_start; bucket_now < bucket->p_end; ++bucket_now) {
            if (this->key_eq(key, bucket_now->key)) {
                return bucket_now->value;
            }
        }

        return std::nullopt;
    }

    void insert(Range<const char> key, val_t value) {
        std::size_t hash = this->hash_let(key);
        Range<HNode> *bucket = this->bucket_list.data() + (hash % cap);

        // insert into empty bucket
        if (bucket->p_start == nullptr) {
            bucket->p_start = new HNode[1];
            bucket->p_end = bucket->p_start + 1;

            this->node_init(bucket->p_start, key, value);

            return;
        }

        // try to manually find
        for (HNode *store_now = bucket->p_start; store_now < bucket->p_end; ++store_now) {
            if (this->key_eq(key, store_now->key)) {
                store_now->value = value;

                return;
            }
        }

        // handle insertion into occupied node
        {
            HNode *next_start = new HNode[bucket->p_end - bucket->p_start + 1];
            HNode *next_now = next_start;
            HNode *bucket_now = bucket->p_start;

            do {
                *(next_now++) = *(bucket_now++);
            } while (bucket_now < bucket->p_end);

            bucket->p_start = next_start;
            bucket->p_end = next_now + 1;

            this->node_init(next_now, key, value);
        }
    }

    void remove(Range<const char> key) {
        std::size_t hash = this->hash_let(key);
        Range<HNode> *bucket = this->bucket_list.data() + (hash % cap);

        if (bucket->p_start == nullptr) {
            return;
        }

        if (bucket->size() == 1) {
            if (!this->key_eq(key, bucket->p_start->key)) {
                return;
            }

            delete[] bucket->p_start->key.p_start;
            delete[] bucket->p_start;

            bucket->p_start = nullptr;
            bucket->p_end = nullptr;

            return;
        }

        HNode *bucket_search = bucket->p_start;

        do {
            if (!this->key_eq(key, bucket_search->key)) {
                ++bucket_search;

                continue;
            }

            HNode *next_start = new HNode[bucket->p_end - bucket->p_start - 1];
            HNode *next_now = next_start;

            for (HNode *bucket_now = bucket->p_start; bucket_now < bucket_search; ++bucket_now) {
                *(next_now++) = *bucket_now;
            }

            for (HNode *bucket_now = bucket_search + 1; bucket_now < bucket->p_end; ++bucket_now) {
                *(next_now++) = *bucket_now;
            }

            delete[] bucket_search->key.p_start;
            delete[] bucket->p_start;

            bucket->p_start = next_start;
            bucket->p_end = next_now;

            return;
        } while (bucket_search < bucket->p_end);
    }

    ~HMapStatic() {
        Range<HNode> *bucket_now = this->bucket_list.data();
        Range<HNode> *bucket_end = bucket_now + this->bucket_list.size();

        for (; bucket_now < bucket_end; ++bucket_now) {
            if (bucket_now->size() >= 1) {
                for (HNode *node_now = bucket_now->p_start; node_now < bucket_now->p_end; ++node_now) {
                    delete[] node_now->key.p_start;
                }

                delete[] bucket_now->p_start;
            }
        }
    }
};
