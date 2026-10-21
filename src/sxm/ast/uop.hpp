#pragma once

namespace sxm {

enum class UnaryOp {
    Pos,
    Neg,
};

sxm::UnaryOp uop_let(const char *section_start);

}
