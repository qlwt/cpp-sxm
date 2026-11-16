#include "cmd/perf.hpp"
#include "sxm/eval/compile/build.hpp"
#include "sxm/eval/compile/declaration.hpp"
#include "sxm/eval/flat/eval.hpp"
#include "sxm/eval/linked/core.hpp"
#include "sxm/eval/linked/eval.hpp"
#include "sxm/eval/pure/build.hpp"
#include "sxm/eval/pure/eval.hpp"
#include "sxm/util/search_map/h_static.hpp"
#include "sxm/eval/compile/core.hpp"
#include "sxm/eval/flat/core.hpp"
#include "sxm/eval/pure/core.hpp"
#include "sxm/parser/parse.hpp"
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

    expr += "(n=1)";

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

void cmd::perf(std::size_t size, bool flag_safe) {
    std::printf("generating expressions\n");
    std::string expr_parse = expr_new(size);
    std::string expr_pure = expr_new(size);
    std::string expr_flat = expr_new(size);
    std::string expr_linked = expr_new(size);
    std::printf("expressions generated\n\n");

    std::size_t parse_duration;

    {
        std::printf("parse::start tokens_size=%llu\n", expr_parse.size());

        parse_duration = exec([&]() -> void {
            const char *src = expr_parse.c_str();

            for (auto token : sxm::parse(src)) {
                if (!token.has_value()) {
                    std::printf("Parser Error: %s\n", token.error().tostr().c_str());

                    break;
                }
            }
        });

        std::printf("parse::done duration=%f\n", (double)parse_duration / 1e6);
        std::printf("\n");
    }

    if (!flag_safe) {
        sxm::HMapStatic<10, sxm::NumValue> varmap {};

        std::printf("eval_pure::start tokens_size=%llu\n", expr_pure.size());

        std::size_t lduration = exec([&]() -> void {
            const char *src = expr_pure.c_str();
            std::expected<sxm::peval::Node *, sxm::ParserError> ast = sxm::peval::build(src);

            if (!ast.has_value()) {
                std::printf("Parser Error: %s\n", ast.error().tostr().c_str());
            } else {
                std::size_t lduration_eval = exec([&]() -> void {
                    std::expected<sxm::NumValue, sxm::EvalError> result = sxm::peval::eval_mut(ast.value(), {
                        .varmap = varmap,
                    });

                    if (!result.has_value()) {
                        std::printf("Eval Error: %s\n", result.error().tostr().c_str());
                    } else {
                        printf("Value is %f\n", result->real);
                    }
                });

                std::printf("eval_pure::calc %.12f\n", (double)lduration_eval / 1e6);

                delete ast.value();
            }
        });

        std::printf("eval_pure::done %f\n", (double)lduration / 1e6);
        std::printf("eval_pure::done::noparse %f\n", (double)(lduration - parse_duration) / 1e6);
        std::printf("\n");
    }

    {
        sxm::HMapStatic<10, sxm::NumValue> varmap {};

        std::printf("eval_flat::start tokens_size=%llu\n", expr_flat.size());

        std::size_t lduration = exec([&]() -> void {
            const char *src = expr_flat.c_str();
            std::expected<sxm::NumValue, sxm::feval::Error> result = sxm::feval::eval_mut(src, {
                .varmap = varmap,
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
        });

        std::printf("eval_flat::done %f\n", (double)lduration / 1e6);
        std::printf("eval_flat::done::noparse %f\n", (double)(lduration - parse_duration) / 1e6);
        std::printf("\n");
    }

    {
        sxm::HMapStatic<10, sxm::NumValue> varmap {};
        std::printf("eval_linked::start tokens_size=%llu\n", expr_linked.size());

        std::size_t lduration = exec([&]() -> void {
            const char *src = expr_linked.c_str();
            std::expected<sxm::NumValue, sxm::leval::Error> result = sxm::leval::eval_mut(src, {
                .varmap = varmap,
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
        });

        std::printf("eval_linked::done %f\n", (double)lduration / 1e6);
        std::printf("eval_linked::done::noparse %f\n", (double)(lduration - parse_duration) / 1e6);
        std::printf("\n");
    }

    {
        sxm::HMapStatic<10, sxm::NumValue> varmap_const {};
        sxm::HMapStatic<10, sxm::ceval::Offset> varmap_state {};
        std::printf("eval_compile::start tokens_size=%llu\n", expr_linked.size());

        std::size_t lduration = exec([&]() -> void {
            const char *src = expr_linked.c_str();

            std::expected<sxm::ceval::Declaration, sxm::ceval::Error> decl = sxm::ceval::build(src);

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
                sxm::NumValue result = decl->eval({});

                printf("Value is %f\n", result.real);
            }
        });

        std::printf("eval_compile::done %f\n", (double)lduration / 1e6);
        std::printf("eval_compile::done::noparse %f\n", (double)(lduration - parse_duration) / 1e6);
        std::printf("\n");
    }
}
