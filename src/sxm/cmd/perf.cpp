#include "sxm/cmd/perf.hpp"
#include "algo/search_map/h_static.hpp"
#include "sxm/debug/print_err_eval.hpp"
#include "sxm/debug/print_err_parser.hpp"
#include "sxm/eval/compile.hpp"
#include "sxm/eval/flat.hpp"
#include "sxm/eval/linked.hpp"
#include "sxm/eval/pure.hpp"
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

void sxm::cmd::perf(std::size_t size, bool flag_safe) {
    std::printf("generating expressions\n");
    std::string expr_parse = expr_new(size);
    std::string expr_pure = expr_new(size);
    std::string expr_flat = expr_new(size);
    std::string expr_linked = expr_new(size);
    std::printf("expressions generated\n\n");

    std::size_t parse_duration;

    {
        std::printf("parse::start tokens_size=%lu\n", expr_parse.size());

        parse_duration = exec([&]() -> void {
            const char *src = expr_parse.c_str();

            for (auto token : sxm::parse(src)) {
                if (!token.has_value()) {
                    sxm::print_err_parser(token.error(), src);

                    break;
                }
            }
        });

        std::printf("parse::done duration=%f\n", (double)parse_duration / 1e6);
        std::printf("\n");
    }

    if (!flag_safe) {
        HMapStatic<10, sxm::NumValue> val_hmap {};

        std::printf("eval_pure::start tokens_size=%lu\n", expr_pure.size());

        std::size_t lduration = exec([&]() -> void {
            const char *src = expr_pure.c_str();
            std::expected<sxm::PEvalNode *, sxm::ParserError> ast = sxm::eval_pure_build(src);

            if (!ast.has_value()) {
                sxm::print_err_parser(ast.error(), src);
            } else {
                std::size_t lduration_eval = exec([&]() -> void {
                    std::expected<sxm::NumValue, sxm::EvalError> result = sxm::eval_pure(ast.value(), &val_hmap);

                    if (!result.has_value()) {
                        sxm::print_err_eval(result.error());
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
        HMapStatic<10, sxm::NumValue> val_hmap {};

        std::printf("eval_flat::start tokens_size=%lu\n", expr_flat.size());

        std::size_t lduration = exec([&]() -> void {
            const char *src = expr_flat.c_str();
            std::expected<sxm::NumValue, sxm::FEvalError> result = sxm::eval_flat(src, &val_hmap);

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
                printf("Value is %f\n", result->real);
            }
        });

        std::printf("eval_flat::done %f\n", (double)lduration / 1e6);
        std::printf("eval_flat::done::noparse %f\n", (double)(lduration - parse_duration) / 1e6);
        std::printf("\n");
    }

    {
        HMapStatic<10, sxm::NumValue> val_hmap {};
        std::printf("eval_linked::start tokens_size=%lu\n", expr_linked.size());

        std::size_t lduration = exec([&]() -> void {
            const char *src = expr_linked.c_str();
            std::expected<sxm::NumValue, sxm::FEvalError> result = sxm::eval_linked(src, &val_hmap);

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
                printf("Value is %f\n", result->real);
            }
        });

        std::printf("eval_linked::done %f\n", (double)lduration / 1e6);
        std::printf("eval_linked::done::noparse %f\n", (double)(lduration - parse_duration) / 1e6);
        std::printf("\n");
    }

    {
        HMapStatic<10, sxm::NumValue> sxmvar_const {};
        HMapStatic<10, sxm::CEvalOffset> sxmvar_state {};
        std::printf("eval_compile::start tokens_size=%lu\n", expr_linked.size());

        std::size_t lduration = exec([&]() -> void {
            const char *src = expr_linked.c_str();

            std::expected<sxm::CEvalDeclaration, sxm::CEvalError> decl = sxm::eval_compile_build(src, {
                .sxmvar_const = &sxmvar_const,
                .sxmvar_state = &sxmvar_state,
                .sxmvar_state_size = 0,
            });

            if (!decl.has_value()) {
                std::visit([src](const auto & error) {
                    using T = std::decay_t<decltype(error)>;

                    if constexpr (std::is_same_v<T, sxm::ParserError>) {
                        sxm::print_err_parser(error, src);
                    } else if constexpr (std::is_same_v<T, sxm::EvalError>) {
                        sxm::print_err_eval(error);
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
