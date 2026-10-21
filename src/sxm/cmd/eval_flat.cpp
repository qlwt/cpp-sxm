#include "sxm/cmd/eval_flat.hpp"
#include "sxm/debug/print_err_eval.hpp"
#include "sxm/debug/print_err_parser.hpp"
#include "sxm/eval/flat.hpp"
#include <expected>
#include <variant>

void sxm::cmd::eval_flat(const char *src, HMap<sxm::NumValue> *val_hmap) {
    std::expected<sxm::NumValue, sxm::FEvalError> result = sxm::eval_flat(src, val_hmap);

    if (!result.has_value()) {
        std::visit([src](const auto &error) {
            using T = std::decay_t<decltype(error)>;

            if constexpr (std::is_same_v<T, sxm::ParserError>) {
                sxm::print_err_parser(error, src);
            } else if constexpr (std::is_same_v<T, sxm::EvalError>) {
                sxm::print_err_eval(error);
            }
        }, result.error());
    } else {
        printf("Value is %f\n", result->real);
    }
}
