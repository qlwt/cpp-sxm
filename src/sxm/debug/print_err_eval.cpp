#include "sxm/debug/print_err_eval.hpp"
#include "sxm/parser/parse.hpp"
#include <cstdio>

void sxm::print_err_eval(sxm::EvalError error) {
    std::printf("Eval Error: %s\n", error.message.c_str());
}
