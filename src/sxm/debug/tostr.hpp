#include "sxm/ast/node.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/parser/parse.hpp"
#include "sxm/parser/token.hpp"
#include <string>

#pragma once

namespace sxm {

std::string evalerr_tostr(sxm::EvalError& error);
std::string parseerr_tostr(sxm::ParserError& error);

std::string token_kind_tostr(sxm::Token_Kind kind);
std::string astnode_kind_tostr(sxm::ASTNode_Kind kind);

std::string uop_tostr(sxm::UnaryOp kind);
std::string biop_tostr(sxm::BinaryOp kind);

}
