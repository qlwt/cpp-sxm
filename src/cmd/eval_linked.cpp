#include "cmd/eval_linked.hpp"
#include "sxm/eval/linked/core.hpp"
#include "sxm/eval/linked/eval.hpp"
#include <expected>
#include <variant>

void cmd::eval_linked(const char *src, sxm::HMap<sxm::NumValue> *varmap) {
    std::expected<sxm::NumValue, sxm::leval::Error> result = sxm::leval::eval_mut(src, {
        .varmap = *varmap
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
