#include "sxm/ast/uop.hpp"
#include <utility>

sxm::UnaryOp sxm::uop_let(const char *section_start) {
    switch (*section_start) {
    case '+':
        return sxm::UnaryOp::Pos;
    case '-':
        return sxm::UnaryOp::Neg;
    }

    std::unreachable();
}

sxm::NumValue sxm::uop_apply(sxm::UnaryOp op, const sxm::NumValue &target) {
    switch (op) {
    case sxm::UnaryOp::Pos: {
        return target;
    }
    case sxm::UnaryOp::Neg: {
        return sxm::NumValue::neg(&target);
    }
    default: {
        std::unreachable();
    }
    }
}
