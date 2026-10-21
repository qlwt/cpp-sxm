#pragma once

namespace sxm {

enum class BinaryOp {
    Add,
    Sub,
    Div,
    Mul,
    Mod,
    Pow,
    Assign,

    Eq,
    UEq,

    GT,
    GTE,

    LT,
    LTE,
};

BinaryOp biop_let(const char *name_start, const char* name_end);
unsigned short biop_priority_let(sxm::BinaryOp biop);

}
