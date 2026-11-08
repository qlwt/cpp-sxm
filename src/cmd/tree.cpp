#include "sxm/error/error.hpp"
#include "sxm/eval/pure/ast.hpp"
#include "sxm/eval/pure/build.hpp"
#include "sxm/eval/pure/core.hpp"
#include "cmd/tree.hpp"

void cmd::tree(const char *src) {
    std::expected<sxm::peval::Node *, sxm::ParserError> ast = sxm::peval::build(src);

    if (!ast.has_value()) {
        std::printf("Parser Error: %s\n", ast.error().tostr().c_str());

        return;
    }

    std::printf("%s\n", sxm::peval::node_tostr(ast.value()).c_str());

    delete ast.value();
}
