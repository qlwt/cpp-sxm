#include "sxm/debug/print_err_parser.hpp"
#include "sxm/parser/parse.hpp"
#include "sxm/cmd/tkn.hpp"

void sxm::cmd::tkn(const char *src) {
    for (auto token : sxm::parse(src)) {
        if (!token.has_value()) {
            sxm::print_err_parser(token.error(), src);

            continue;
        }

        switch (token->kind) {
        case sxm::Token_Kind::End: {
            std::printf("\n");

            break;
        }
        case sxm::Token_Kind::Space:
            break;
        case sxm::Token_Kind::FnOpen: {
            std::printf("(");

            break;
        }
        case sxm::Token_Kind::FnClose: {
            std::printf(")");

            break;
        }
        case sxm::Token_Kind::PriorityOpen: {
            std::printf("[");

            break;
        }
        case sxm::Token_Kind::PriorityClose: {
            std::printf("]");

            break;
        }
        case sxm::Token_Kind::FnComma: {
            std::printf(", ");

            break;
        }
        case sxm::Token_Kind::UnaryOp:
        case sxm::Token_Kind::NumberInt:
        case sxm::Token_Kind::NumberReal:
        case sxm::Token_Kind::Reference: {
            std::printf("%.*s", static_cast<int>(token->section_end - token->section_start), token->section_start);

            break;
        }
        case sxm::Token_Kind::BinaryOp: {
            std::printf(" %.*s ", static_cast<int>(token->section_end - token->section_start), token->section_start);

            break;
        }
        }
    }
}
