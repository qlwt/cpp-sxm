#include "sxm/parser/token.hpp"
#include <cstddef>
#include <expected>
#include <generator>

#pragma once

namespace sxm {

enum class EvalError_Kind {
    FnEval,
    VarUndef,
    FnUndef,
    NodeUnexp,
    MemoryError,
};

struct EvalError {
    EvalError_Kind kind;
    std::string message;
};

enum class ParserError_Kind {
    NoMatch,
    ScopeMismatch,
};

struct ParserError {
    ParserError_Kind kind;
    std::size_t pointer;
};

std::generator<std::expected<sxm::Token, sxm::ParserError> &> parse(const char *src);

}
