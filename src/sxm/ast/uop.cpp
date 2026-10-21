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
