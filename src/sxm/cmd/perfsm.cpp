#include "sxm/cmd/perfsm.hpp"
#include "algo/search_map/h_static.hpp"
#include "sxm/debug/print_err_eval.hpp"
#include "sxm/debug/print_err_parser.hpp"
#include "sxm/eval/compile.hpp"
#include "sxm/eval/pure.hpp"
#include "sxm/parser/parse.hpp"
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

void sxm::cmd::perfsm(std::size_t size, std::size_t repeats) {
    std::string expr_pure = expr_new(size);
    std::string expr_compile_decl = expr_pure;
    std::string expr_compile_unit = expr_pure;

    std::printf("tokens_size=%lu\n", expr_pure.size());

    {
        const char *src = expr_pure.c_str();
        std::expected<sxm::PEvalNode *, sxm::ParserError> ast = sxm::eval_pure_build(src);

        if (!ast.has_value()) {
            sxm::print_err_parser(ast.error(), src);
        } else {
            bool has_error { false };

            std::size_t lduration = exec([&]() -> void {
                for (std::size_t repeats_left = repeats; repeats_left-- > 0;) {
                    HMapStatic<10, sxm::NumValue> val_hmap {};

                    val_hmap.insert({ "n" }, { static_cast<double>(repeats_left) });

                    std::expected<sxm::NumValue, sxm::EvalError> result = sxm::eval_pure(ast.value(), &val_hmap);

                    if (!result.has_value()) {
                        has_error = true;

                        sxm::print_err_eval(result.error());

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

        HMapStatic<32, sxm::CEvalOffset> sxmvar_local {};

        sxmvar_local.insert({ "n" }, 0);

        std::expected<sxm::CEvalDeclaration, sxm::CEvalError> decl = sxm::eval_compile_build(src, {
            .sxmvar_const {},
            .sxmvar_state = &sxmvar_local,
            .sxmvar_state_size = 1
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
            std::size_t lduration = exec([&]() -> void {
                for (std::size_t repeats_left = repeats; repeats_left-- > 0;) {
                    sxm::NumValue sxmvar_local[1] {
                        { static_cast<double>(repeats_left) },
                    };

                    decl->eval({ sxmvar_local, sxmvar_local + 1, });
                }
            });

            std::printf("eval_compile_decl::done %f\n", (double)lduration / 1e6);
            std::printf("\n");
        }
    }

    {
        const char *src = expr_compile_unit.c_str();

        HMapStatic<32, sxm::CEvalOffset> sxmvar_local {};

        sxmvar_local.insert({ "n" }, 0);

        std::expected<sxm::CEvalDeclaration, sxm::CEvalError> decl = sxm::eval_compile_build(src, {
            .sxmvar_const {},
            .sxmvar_state = &sxmvar_local,
            .sxmvar_state_size = 1
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
            sxm::CEvalUnit unit = decl->init();

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
