#include "algo/search_map/h.hpp"
#include "algo/search_map/i.hpp"
#include <optional>

#pragma once

template <typename val_t>

class HMapFallback : public HMap<val_t> {
  public:
    IMap<val_t> *back;
    HMap<val_t> *front;

    HMapFallback(IMap<val_t> *back, HMap<val_t> *front)
        : back(back), front(front) {}

    std::size_t hash_let(Range<const char> key) const override {
        return this->front->hash_let(key);
    }

    std::optional<val_t> find(Range<const char> key) const override {
        std::optional<val_t> front_value = this->front->find(key);

        if (front_value.has_value()) {
            return front_value.value();
        }

        return this->back->find(key);
    }

    void insert(Range<const char> key, val_t value) override {
        this->front->insert(key, value);
    }

    void remove(Range<const char> key) override {
        this->front->remove(key);
    }
};
