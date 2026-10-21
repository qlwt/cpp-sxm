#include "sxm/ast/biop.hpp"
#include <utility>

sxm::BinaryOp sxm::biop_let(const char *section_start, const char* section_end) {
    switch (*section_start) {
    case '+':
        return sxm::BinaryOp::Add;
    case '-':
        return sxm::BinaryOp::Sub;
    case '/':
        return sxm::BinaryOp::Div;
    case '*':
        return sxm::BinaryOp::Mul;
    case '^':
        return sxm::BinaryOp::Pow;
    case '%':
        return sxm::BinaryOp::Mod;
    case '>': {
        const char *section_next = section_start + 1;

        if (section_next < section_end && *section_next == '=') {
            return sxm::BinaryOp::GTE;
        }

        return sxm::BinaryOp::GT;
    }
    case '<': {
        const char *section_next = section_start + 1;

        if (section_next < section_end && *section_next == '=') {
            return sxm::BinaryOp::LTE;
        }

        return sxm::BinaryOp::LT;
    }
    case '=': {
        const char *section_next = section_start + 1;

        if (section_next < section_end && *section_next == '=') {
            return sxm::BinaryOp::Eq;
        }

        return sxm::BinaryOp::Assign;
    }
    case '!': {
        const char *section_next = section_start + 1;

        if (section_next < section_end && *section_next == '=') {
            return sxm::BinaryOp::UEq;
        }

        break;
    }
    }

    std::unreachable();
}

unsigned short sxm::biop_priority_let(sxm::BinaryOp biop) {
    switch (biop) {
    case sxm::BinaryOp::Eq:
    case sxm::BinaryOp::UEq:
    case sxm::BinaryOp::GT:
    case sxm::BinaryOp::GTE:
    case sxm::BinaryOp::LT:
    case sxm::BinaryOp::LTE:
        return 0;
    case sxm::BinaryOp::Assign:
        return 1;
    case sxm::BinaryOp::Add:
    case sxm::BinaryOp::Sub:
        return 2;
    case sxm::BinaryOp::Mul:
    case sxm::BinaryOp::Div:
    case sxm::BinaryOp::Mod:
        return 3;
    case sxm::BinaryOp::Pow:
        return 4;
    }

    std::unreachable();
}
