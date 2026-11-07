#include "algo/search_map/h.hpp"
#include "sxm/ast/numvalue.hpp"

#pragma once

namespace sxm::cmd {

void eval_compile(const char *src, HMap<sxm::NumValue> *val_hmap);

}
