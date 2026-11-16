#include "cmd/bytecode.hpp"
#include "sxm/error/error.hpp"
#include "sxm/util/search_map/h_static.hpp"
#include "sxm/eval/compile/core.hpp"
#include "sxm/eval/compile/build.hpp"
#include "sxm/eval/compile/declaration.hpp"
#include <expected>
#include <format>
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
        std::printf("compiled successfully %llu instructions\n", result->instr_list.size());

        for (auto instr_now : result->instr_list) {
            switch (instr_now.kind) {
            case sxm::ceval::Instr_Kind::Ret: {
                std::printf("return %s\n", offset_parse(instr_now.left.offset).c_str());

                break;
            }

            case sxm::ceval::Instr_Kind::Call: {
                std::printf("call %p %llu %s\n", (void *)instr_now.left.fn, instr_now.right.length, offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::ArgPush: {
                std::printf("arg %s\n", offset_parse(instr_now.left.offset).c_str());

                break;
            }

            case sxm::ceval::Instr_Kind::Set: {
                std::printf("set %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::Not: {
                std::printf("not %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::Neg: {
                std::printf("neg %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }

            case sxm::ceval::Instr_Kind::Conj: {
                std::printf("conj %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::Disj: {
                std::printf("disj %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::Add: {
                std::printf("add %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::Sub: {
                std::printf("sub %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::Mul: {
                std::printf("mul %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::Div: {
                std::printf("div %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::Mod: {
                std::printf("mod %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::Pow: {
                std::printf("pow %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::Eq: {
                std::printf("eq %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::UEq: {
                std::printf("ueq %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::GTE: {
                std::printf("gte %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::LTE: {
                std::printf("lte %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::LT: {
                std::printf("lt %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::GT: {
                std::printf("gt %s %s %s\n", offset_parse(instr_now.left.offset).c_str(), offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            case sxm::ceval::Instr_Kind::Assign: {
                std::printf("ass %s %s\n", offset_parse(instr_now.right.offset).c_str(), offset_parse(instr_now.output.offset).c_str());

                break;
            }
            }
        }
    }
}
