#include "sxm/cmd/assemble.hpp"
#include "algo/search_map/h_static.hpp"
#include "sxm/debug/print_err_eval.hpp"
#include "sxm/debug/print_err_parser.hpp"
#include "sxm/eval/compile.hpp"
#include <expected>
#include <format>
#include <print>
#include <string>
#include <variant>

namespace {

inline std::string offset_parse(sxm::CEvalOffset offset) {
    return std::format("{}:{}", offset.flag_const() ? "const" : "state", offset.get());
}

}

void sxm::cmd::assemble(const char *src, HMap<sxm::NumValue> *val_hmap) {
    HMapStatic<32, sxm::CEvalOffset> sxmvar_local {};

    sxmvar_local.insert({ "l0" }, 0);
    sxmvar_local.insert({ "l1" }, 1);
    sxmvar_local.insert({ "l2" }, 2);

    std::expected<sxm::CEvalDeclaration, sxm::CEvalError> result = sxm::eval_compile_build(src, {
        .sxmvar_const = val_hmap,
        .sxmvar_state = &sxmvar_local,
        .sxmvar_state_size = 3,
    });

    if (!result.has_value()) {
        std::visit([src](const auto & error) {
            using T = std::decay_t<decltype(error)>;

            if constexpr (std::is_same_v<T, sxm::ParserError>) {
                sxm::print_err_parser(error, src);
            } else if constexpr (std::is_same_v<T, sxm::EvalError>) {
                sxm::print_err_eval(error);
            }
        }, result.error());
    } else {
        std::printf("compiled successfully %lu\n", result->inst_list.size());

        for (auto inst_now : result->inst_list) {
            switch (inst_now.kind) {
            case sxm::CEvalInstr_Kind::Ret: {
                std::println("return {}", offset_parse(inst_now.left.offset));

                break;
            }

            case sxm::CEvalInstr_Kind::Call: {
                std::println("call {} {} {}", (void *)inst_now.left.fn, inst_now.right.length, offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::ArgPush: {
                std::println("arg {}", offset_parse(inst_now.left.offset));

                break;
            }

            case sxm::CEvalInstr_Kind::Set: {
                std::println("set {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::Neg: {
                std::println("neg {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.output.offset));

                break;
            }

            case sxm::CEvalInstr_Kind::Add: {
                std::println("add {} {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::Sub: {
                std::println("sub {} {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::Mul: {
                std::println("mul {} {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::Div: {
                std::println("div {} {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::Mod: {
                std::println("mod {} {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::Pow: {
                std::println("pow {} {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::Eq: {
                std::println("eq {} {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::UEq: {
                std::println("ueq {} {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::GTE: {
                std::println("gte {} {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::LTE: {
                std::println("lte {} {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::LT: {
                std::println("lt {} {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::GT: {
                std::println("gt {} {} {}", offset_parse(inst_now.left.offset), offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            case sxm::CEvalInstr_Kind::Assign: {
                std::println("ass {} {}", offset_parse(inst_now.right.offset), offset_parse(inst_now.output.offset));

                break;
            }
            default: {
                std::unreachable();
            }
            }
        }
    }
}
