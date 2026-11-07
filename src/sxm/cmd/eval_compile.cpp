#include "sxm/cmd/eval_compile.hpp"
#include "algo/search_map/h_static.hpp"
#include "sxm/debug/print_err_eval.hpp"
#include "sxm/debug/print_err_parser.hpp"
#include "sxm/eval/compile.hpp"
#include <expected>
#include <variant>

void sxm::cmd::eval_compile(const char *src, HMap<sxm::NumValue> *val_hmap) {
    HMapStatic<32, sxm::CEvalOffset> sxmvar_local {};

    sxmvar_local.insert({ "l0" }, 0);
    sxmvar_local.insert({ "l1" }, 1);
    sxmvar_local.insert({ "l2" }, 2);

    std::expected<sxm::CEvalDeclaration, sxm::CEvalError> unit_decl = sxm::eval_compile_build(src, {
        .sxmvar_const = val_hmap,
        .sxmvar_state = &sxmvar_local,
        .sxmvar_state_size = 3,
    });

    if (!unit_decl.has_value()) {
        std::visit([src](const auto & error) {
            using T = std::decay_t<decltype(error)>;

            if constexpr (std::is_same_v<T, sxm::ParserError>) {
                sxm::print_err_parser(error, src);
            } else if constexpr (std::is_same_v<T, sxm::EvalError>) {
                sxm::print_err_eval(error);
            }
        }, unit_decl.error());
    } else {
        sxm::NumValue local_list[3] = { {0.0}, { 1.0 }, { 2.0 }, };

        sxm::NumValue result = unit_decl->eval({
            local_list,
            local_list + (sizeof(local_list) / sizeof(sxm::NumValue)),
        });

        std::printf("Result is %f\n", result.real);

        // sxm::CEvalUnit unit = eval_compile_init(unit_decl.value());
        // sxm::NumValue result = eval_compile_unit(unit);
        //
        // std::printf("Result is %f\n", result.real);
    }
}
