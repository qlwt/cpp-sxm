#include "cmd/perfsm.hpp"
#include "sxm/eval/compile/build.hpp"
#include "sxm/eval/compile/declaration.hpp"
#include "sxm/eval/pure/build.hpp"
#include "sxm/eval/pure/eval.hpp"
#include "sxm/util/search_map/h_static.hpp"
#include "sxm/eval/compile/core.hpp"
#include "sxm/eval/pure/core.hpp"
#include "sxm/util/search_map/s_nullmap.hpp"
#include <cstddef>
#include <random>
#include <string>
#include <chrono>

namespace {

std::mt19937 generator {1000};
std::uniform_int_distribution<std::size_t> distr_symbol{0, 100};
std::uniform_int_distribution<std::size_t> distr_size{0, 2};
std::uniform_int_distribution<std::size_t> distr_expr{0, 8};
std::uniform_int_distribution<std::size_t> distr_operator{0, 3};

void push_symbol(std::string *target) {
    for (std::size_t now = distr_size(generator); now-- > 0;) {
        target->push_back('-');
    }

    if (distr_symbol(generator) >= 1) {
        target->push_back('1');
    } else {
        target->push_back('n');
    }
}

void push_op(std::string *target) {
    switch (distr_operator(generator)) {
    case 0: {
        target->push_back('+');

        break;
    }
    case 1: {
        target->push_back('*');

        break;
    }
    case 2: {
        target->push_back('>');

        break;
    }
    case 3: {
        target->push_back('=');

        break;
    }
    }
}

std::string expr_new(std::size_t length) {
    std::string expr {};
    std::size_t depth {0};

    expr += "1";

    for (; length-- > 0;) {
        unsigned long kind = distr_expr(generator);

        switch (kind) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4: {
            push_op(&expr);
            push_symbol(&expr);

            break;
        }
        case 5: {
            push_op(&expr);
            expr.push_back('(');
            push_symbol(&expr);

            depth += 1;

            break;
        }
        case 6: {
            push_op(&expr);
            expr += "b(";

            std::size_t arg_size = distr_size(generator);

            if (arg_size == 0) {
                expr.push_back(')');
            } else {
                for (std::size_t i = 1; i < arg_size; ++i) {
                    push_symbol(&expr);
                    expr.push_back(',');
                }

                push_symbol(&expr);

                depth += 1;
            }

            break;
        }
        case 7:
        case 8: {
            if (depth >= 1) {
                --depth;

                expr.push_back(')');
            }

            break;
        }
        }
    }

    for (; depth-- > 0;) {
        expr.push_back(')');
    }

    return expr;
}

template <typename T>
std::size_t exec(T cb) {
    auto start = std::chrono::steady_clock::now();

    cb();

    auto end = std::chrono::steady_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

}

void cmd::perfsm(std::size_t size, std::size_t repeats) {
    std::string expr_pure = expr_new(size);
    std::string expr_compile_decl = expr_pure;
    std::string expr_compile_unit = expr_pure;

    std::printf("tokens_size=%lu\n", expr_pure.size());

    {
        const char *src = expr_pure.c_str();
        std::expected<sxm::peval::Node *, sxm::ParserError> ast = sxm::peval::build(src);

        if (!ast.has_value()) {
            std::printf("Parser Error: %s\n", ast.error().tostr().c_str());
        } else {
            bool has_error { false };

            std::size_t lduration = exec([&]() -> void {
                for (std::size_t repeats_left = repeats; repeats_left-- > 0;) {
                    sxm::HMapStatic<10, sxm::NumValue> varmap {};

                    varmap.insert({ "n" }, { static_cast<double>(repeats_left) });

                    std::expected<sxm::NumValue, sxm::EvalError> result = sxm::peval::eval_mut(ast.value(), {
                        .varmap = varmap,
                    });

                    if (!result.has_value()) {
                        has_error = true;

                        std::printf("Eval Error: %s\n", result.error().tostr().c_str());

                        return;
                    }
                }
            });

            delete ast.value();

            std::printf("eval_pure::done %f\n", (double)lduration / 1e6);
            std::printf("\n");
        }
    }

    {
        const char *src = expr_compile_decl.c_str();

        sxm::HMapStatic<32, sxm::ceval::Offset> varmap_local {};

        varmap_local.insert({ "n" }, 0);

        std::expected<sxm::ceval::Declaration, sxm::ceval::Error> decl = sxm::ceval::build(src, {
            .varmap_const = sxm::SMapNull<sxm::NumValue>(),
            .varmap_state = varmap_local,
            .varmap_state_size = 1
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
            std::size_t lduration = exec([&]() -> void {
                for (std::size_t repeats_left = repeats; repeats_left-- > 0;) {
                    sxm::NumValue varmap_local[1] {
                        { static_cast<double>(repeats_left) },
                    };

                    decl->eval({ varmap_local, varmap_local + 1, });
                }
            });

            std::printf("eval_compile_decl::done %f\n", (double)lduration / 1e6);
            std::printf("\n");
        }
    }

    {
        const char *src = expr_compile_unit.c_str();

        sxm::HMapStatic<32, sxm::ceval::Offset> varmap_local {};

        varmap_local.insert({ "n" }, 0);

        std::expected<sxm::ceval::Declaration, sxm::ceval::Error> decl = sxm::ceval::build(src, {
            .varmap_const = sxm::SMapNull<sxm::NumValue>(),
            .varmap_state = varmap_local,
            .varmap_state_size = 1
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
            sxm::ceval::Unit unit = decl->init();

            std::size_t lduration = exec([&]() -> void {
                for (std::size_t repeats_left = repeats; repeats_left-- > 0;) {
                    unit.state_list.p_start[0] = sxm::NumValue { (double)repeats_left };

                    unit.eval();
                }
            });

            std::printf("eval_compile_unit::done %f\n", (double)lduration / 1e6);
            std::printf("\n");
        }
    }
}
