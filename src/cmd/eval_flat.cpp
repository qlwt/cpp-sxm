#include "cmd/eval_flat.hpp"
#include "sxm/eval/flat/core.hpp"
#include "sxm/eval/flat/eval.hpp"
#include <expected>
#include <variant>

void cmd::eval_flat(const char *src, sxm::HMap<sxm::NumValue> *varmap) {
    std::expected<sxm::NumValue, sxm::feval::Error> result = sxm::feval::eval_mut(src, {
        .varmap = *varmap,
    });

    if (!result.has_value()) {
        std::visit([](const auto & error) {
            using T = std::decay_t<decltype(error)>;

            if constexpr (std::is_same_v<T, sxm::ParserError>) {
                std::printf("Parser Error: %s\n", error.tostr().c_str());
            } else if constexpr (std::is_same_v<T, sxm::EvalError>) {
                std::printf("Eval Error: %s\n", error.tostr().c_str());
            }
        }, result.error());
    } else {
        printf("Value is %f\n", result->real);
    }
}
