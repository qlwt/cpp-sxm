#include "cmd/eval_pure.hpp"
#include "sxm/error/error.hpp"
#include "sxm/eval/pure/core.hpp"
#include "sxm/eval/pure/build.hpp"
#include "sxm/eval/pure/eval.hpp"
#include <expected>

void cmd::eval_pure(const char *src, sxm::HMap<sxm::NumValue> *varmap) {
    std::expected<sxm::peval::Node *, sxm::ParserError> ast = sxm::peval::build(src);

    if (!ast.has_value()) {
        std::printf("Parser Error: %s\n", ast.error().tostr().c_str());

        return;
    }

    std::expected<sxm::NumValue, sxm::EvalError> result = sxm::peval::eval_mut(ast.value(), {
        .varmap = *varmap
    });

    delete ast.value();

    if (!result.has_value()) {
        std::printf("Eval Error: %s\n", result.error().tostr().c_str());
    } else {
        printf("Value is %f\n", result->real);
    }
}
