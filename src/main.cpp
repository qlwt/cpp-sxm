#include "algo/search_map/h_static.hpp"
#include "sxm/cmd/assemble.hpp"
#include "sxm/cmd/perf.hpp"
#include "sxm/ast/numvalue.hpp"
#include <charconv>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <math.h>
#include <string>
#include "sxm/cmd/perfsm.hpp"
#include "sxm/debug/print_ast.hpp"
#include "sxm/debug/valid_let.hpp"
#include "sxm/cmd/tkn.hpp"
#include "sxm/cmd/tree.hpp"
#include "sxm/cmd/eval_pure.hpp"
#include "sxm/cmd/eval_flat.hpp"
#include "sxm/cmd/eval_compile.hpp"
#include "sxm/cmd/eval_linked.hpp"
#include <charconv>

int main() {
    static constexpr std::string_view cmd_exit = ":exit";
    static constexpr std::string_view cmd_tkn = ":tkn ";
    static constexpr std::string_view cmd_asm = ":asm ";
    static constexpr std::string_view cmd_tree = ":tree ";
    static constexpr std::string_view cmd_perf = ":perf ";
    static constexpr std::string_view cmd_perfsm = ":perfsm ";
    static constexpr std::string_view cmd_perf_safe = ":perf_safe ";
    static constexpr std::string_view cmd_linked = ":el ";
    static constexpr std::string_view cmd_flat = ":ef ";
    static constexpr std::string_view cmd_pure = ":ep ";
    static constexpr std::string_view cmd_compile = ":ec ";

    static HMapStatic<255, sxm::NumValue> val_hmap {};

    val_hmap.insert({ "e" }, sxm::NumValue { std::numbers::e });
    val_hmap.insert({ "pi" }, sxm::NumValue { std::numbers::pi });
    val_hmap.insert({ "phi" }, sxm::NumValue { std::numbers::phi });

    while (true) {
        std::printf("Enter Command or Formula: ");

        std::string line;
        std::getline(std::cin, line);

        if (line == cmd_exit) {
            break;
        }

        if (line.starts_with(cmd_perf)) {
            double size {0};

            std::from_chars(line.c_str() + cmd_perf.size(), line.c_str() + line.size(), size);

            if (!std::isnan(size) && !std::isinf(size) && size >= 1) {
                sxm::cmd::perf((std::size_t)size, false);
            } else {
                std::printf("size should be >= 1\n");
            }
        } else if (line.starts_with(cmd_perf_safe)) {
            double size {0};

            std::from_chars(line.c_str() + cmd_perf_safe.size(), line.c_str() + line.size(), size);

            if (!std::isnan(size) && !std::isinf(size) && size >= 1) {
                sxm::cmd::perf((std::size_t)size, true);
            } else {
                std::printf("size should be >= 1\n");
            }
        } else if (line.starts_with(cmd_perfsm)) {
            const char *src = line.c_str() + cmd_perfsm.size();
            double size;
            double repeats;

            const char *delim_end;
            const char *delim_idx {};

            for (const char *arg_now = src;;) {
                if (*arg_now == '\0') {
                    delim_end = arg_now;

                    break;
                }

                if (*arg_now == '*') {
                    delim_idx = arg_now;
                }

                ++arg_now;
            }

            if (delim_idx == nullptr) {
                std::printf("Bad argument format. Expected {token_size}*{repeats}\n");

                return 1;
            }

            auto parser_amount = std::from_chars(src, delim_idx, size);
            auto parser_ratio = std::from_chars(delim_idx + 1, delim_end, repeats);

            if (parser_amount.ec != std::errc {} || parser_ratio.ec != std::errc {}) {
                if (delim_idx == nullptr) {
                    std::printf("Bad argument format. Expected {token_size}*{repeats}\n");

                    return 1;
                }
            }

            std::printf("size: %lu repeats: %lu\n", static_cast<std::size_t>(size), static_cast<std::size_t>(repeats));

            sxm::cmd::perfsm(size, repeats);
        } else if (line.starts_with(cmd_tkn)) {
            sxm::cmd::tkn(line.c_str() + cmd_tkn.size());
        } else if (line.starts_with(cmd_tree)) {
            sxm::cmd::tree(line.c_str() + cmd_tree.size());
        } else if (line.starts_with(cmd_linked)) {
            sxm::cmd::eval_linked(line.c_str() + cmd_linked.size(), &val_hmap);
        } else if (line.starts_with(cmd_asm)) {
            sxm::cmd::assemble(line.c_str() + cmd_asm.size(), &val_hmap);
        } else if (line.starts_with(cmd_pure)) {
            sxm::cmd::eval_pure(line.c_str() + cmd_pure.size(), &val_hmap);
        } else if (line.starts_with(cmd_flat)) {
            sxm::cmd::eval_flat(line.c_str() + cmd_flat.size(), &val_hmap);
        } else if (line.starts_with(cmd_compile)) {
            sxm::cmd::eval_compile(line.c_str() + cmd_flat.size(), &val_hmap);
        } else {
            sxm::cmd::eval_flat(line.c_str(), &val_hmap);
        }
    }

    return 0;
}
