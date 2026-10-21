#include "sxm/debug/print_err_parser.hpp"
#include "sxm/parser/parse.hpp"
#include <cstdio>
#include <utility>

void sxm::print_err_parser(sxm::ParserError error, const char *src) {
    std::printf("Parsing Error at position %lu:", error.pointer);

    switch (error.kind) {
    case sxm::ParserError_Kind::NoMatch: {
        std::printf(" No Match");

        break;
    }
    case sxm::ParserError_Kind::ScopeMismatch: {
        std::printf(" Scope Mismatch");

        break;
    }
    default: {
        std::unreachable();
    }
    }

    {
        std::printf(" \"");
        if (error.pointer >= 5) {
            std::printf("%.*s", 5, src + error.pointer - 5);
        } else {
            std::printf("%.*s", (int)error.pointer, src);
        }

        std::printf("||error||%.*s\"\n", 10, src + error.pointer);
    }
}
