#include "sxm/cmd/eval_pure.hpp"
#include "sxm/debug/print_err_eval.hpp"
#include "sxm/debug/print_err_parser.hpp"
#include "sxm/eval/pure.hpp"
#include "sxm/parser/parse.hpp"
#include <expected>

void sxm::cmd::eval_pure(const char *src, HMap<sxm::NumValue> *val_hmap) {
    std::expected<sxm::PEvalNode *, sxm::ParserError> ast = sxm::eval_pure_build(src);

    if (!ast.has_value()) {
        sxm::print_err_parser(ast.error(), src);

        return;
    }

    std::expected<sxm::NumValue, sxm::EvalError> result = sxm::eval_pure(ast.value(), val_hmap);

    delete ast.value();

    if (!result.has_value()) {
        sxm::print_err_eval(result.error());

        std::printf("%p\n", ast.value());
    } else {
        printf("Value is %f\n", result->real);
    }
}
