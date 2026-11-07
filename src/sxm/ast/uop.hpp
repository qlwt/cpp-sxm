#pragma once

#include "sxm/ast/numvalue.hpp"
namespace sxm {

enum class UnaryOp {
    Pos,
    Neg,
};

sxm::UnaryOp uop_let(const char *section_start);
sxm::NumValue uop_apply(sxm::UnaryOp op, const sxm::NumValue &target);

}
