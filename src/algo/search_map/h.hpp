#include "algo/list/range.hpp"
#include <cstddef>
#include <cstdio>
#include <optional>
#include "algo/search_map/i.hpp"

#pragma once

template <typename val_t>

class HMap : public IMap<val_t> {
  public:
    virtual std::size_t hash_let(Range<const char>) const = 0;
    virtual std::optional<val_t> find(Range<const char>) const = 0;
    virtual void insert(Range<const char>, val_t value) = 0;
    virtual void remove(Range<const char>) = 0;
};
