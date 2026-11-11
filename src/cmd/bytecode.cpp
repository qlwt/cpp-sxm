#include "cmd/bytecode.hpp"
#include "sxm/error/error.hpp"
#include "sxm/util/search_map/h_static.hpp"
#include "sxm/eval/compile/core.hpp"
#include "sxm/eval/compile/build.hpp"
#include "sxm/eval/compile/declaration.hpp"
#include <expected>
#include <format>
#include <print>
#include <string>
#include <variant>

namespace {

inline std::string offset_parse(sxm::ceval::Offset offset) {
    return std::format("{}:{}", offset.flag_const() ? "const" : "state", offset.get());
}

}

void cmd::bytecode(const char *src, sxm::HMap<sxm::NumValue> *varmap) {
    sxm::HMapStatic<32, sxm::ceval::Offset> varmap_local {};

    varmap_local.insert({ "l" }, 0);

    std::expected<sxm::ceval::Declaration, sxm::ceval::Error> result = sxm::ceval::build(src, {
        .varmap_const = *varmap,
        .varmap_state = varmap_local,
        .varmap_state_size = 1,
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
        std::printf("compiled successfully %lu instructions\n", result->instr_list.size());

        for (auto instr_now : result->instr_list) {
            switch (instr_now.kind) {
            case sxm::ceval::Instr_Kind::Ret: {
                std::println("return {}", offset_parse(instr_now.left.offset));

                break;
            }

            case sxm::ceval::Instr_Kind::Call: {
                std::println("call {} {} {}", (void *)instr_now.left.fn, instr_now.right.length, offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::ArgPush: {
                std::println("arg {}", offset_parse(instr_now.left.offset));

                break;
            }

            case sxm::ceval::Instr_Kind::Set: {
                std::println("set {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::Not: {
                std::println("not {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::Neg: {
                std::println("neg {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.output.offset));

                break;
            }

            case sxm::ceval::Instr_Kind::Conj: {
                std::println("conj {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::Disj: {
                std::println("disj {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::Add: {
                std::println("add {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::Sub: {
                std::println("sub {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::Mul: {
                std::println("mul {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::Div: {
                std::println("div {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::Mod: {
                std::println("mod {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::Pow: {
                std::println("pow {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::Eq: {
                std::println("eq {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::UEq: {
                std::println("ueq {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::GTE: {
                std::println("gte {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::LTE: {
                std::println("lte {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::LT: {
                std::println("lt {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::GT: {
                std::println("gt {} {} {}", offset_parse(instr_now.left.offset), offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            case sxm::ceval::Instr_Kind::Assign: {
                std::println("ass {} {}", offset_parse(instr_now.right.offset), offset_parse(instr_now.output.offset));

                break;
            }
            }
        }
    }
}
