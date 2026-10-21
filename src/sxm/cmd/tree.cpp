#include "sxm/debug/print_err_parser.hpp"
#include "sxm/eval/pure.hpp"
#include "sxm/parser/parse.hpp"
#include "sxm/cmd/tree.hpp"
#include "sxm/debug/print_ast.hpp"

void sxm::cmd::tree(const char *src) {
    std::expected<sxm::PEvalNode *, sxm::ParserError> ast = sxm::eval_pure_build(src);

    if (!ast.has_value()) {
        sxm::print_err_parser(ast.error(), src);

        return;
    }

    sxm::print_ast(ast.value());

    delete ast.value();
}
