#include "cmd/eval_compile.hpp"
#include "sxm/error/error.hpp"
#include "sxm/eval/compile/build.hpp"
#include "sxm/eval/compile/core.hpp"
#include "sxm/util/search_map/h_static.hpp"
#include <expected>
#include <variant>

void cmd::eval_compile(const char *src, sxm::HMap<sxm::NumValue> *varmap) {
    sxm::HMapStatic<32, sxm::ceval::Offset> varmap_local {};

    varmap_local.insert({ "l" }, 0);

    std::expected<sxm::ceval::Declaration, sxm::ceval::Error> decl = sxm::ceval::build(src, {
        .varmap_const = *varmap,
        .varmap_state = varmap_local,
        .varmap_state_size = 1,
    });

    if (!decl.has_value()) {
        std::visit([](const auto & error) {
            using T = std::decay_t<decltype(error)>;

            if constexpr (std::is_same_v<T, sxm::ParserError>) {
                std::printf("Parser Error: %s\n", error.tostr().c_str());
            } else if constexpr (std::is_same_v<T, sxm::EvalError>) {
                std::printf("Eval Error: %s\n", error.tostr().c_str());
            }
        }, decl.error());
    } else {
        sxm::NumValue result = decl->eval({ {0.0} });

        std::printf("Result is %f\n", result.real);
    }
}
