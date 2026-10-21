#include "algo/list/range.hpp"
#include <cstddef>
#include <optional>

#pragma once

template <typename val_t>

class IMap {
  public:
    virtual std::size_t hash_let(Range<const char>) const = 0;
    virtual std::optional<val_t> find(Range<const char>) const = 0;
};
