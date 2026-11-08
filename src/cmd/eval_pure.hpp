#include "sxm/util/search_map/h.hpp"
#include "sxm/ast/numvalue.hpp"

#pragma once

namespace cmd {

void eval_pure(const char *src, sxm::HMap<sxm::NumValue> *val_hmap);

}
