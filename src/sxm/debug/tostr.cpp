#include "sxm/debug/tostr.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/parser/parse.hpp"
#include <stdexcept>
#include <string>

std::string sxm::evalerr_tostr(sxm::EvalError& error) {
    return error.message;
}

std::string sxm::parseerr_tostr(sxm::ParserError& error) {
    switch (error.kind) {
    case sxm::ParserError_Kind::NoMatch:
        return "Could not find match at position " + std::to_string(error.pointer);
    case sxm::ParserError_Kind::ScopeMismatch:
        return "Scope Mismatch Found";
    }

    return "UNKNOWN PARSER ERROR";
}

std::string sxm::token_kind_tostr(sxm::Token_Kind kind) {
    switch (kind) {
    case sxm::Token_Kind::End:
        return "end";
    case sxm::Token_Kind::Space:
        return "space";
    case sxm::Token_Kind::NumberReal:
        return "numbe-real";
    case sxm::Token_Kind::NumberInt:
        return "number-int";
    case sxm::Token_Kind::Reference:
        return "reference";
    case sxm::Token_Kind::BinaryOp:
        return "binary-op";
    case sxm::Token_Kind::UnaryOp:
        return "unary-op";
    case sxm::Token_Kind::PriorityOpen:
        return "priority-open";
    case sxm::Token_Kind::PriorityClose:
        return "priority-close";
    case sxm::Token_Kind::FnOpen:
        return "fn-open";
    case sxm::Token_Kind::FnClose:
        return "fn-close";
    case sxm::Token_Kind::FnComma:
        return "fn-comma";
    }

    return "UNKNOWN TOKEN";
}

std::string sxm::astnode_kind_tostr(sxm::ASTNode_Kind kind) {
    switch (kind) {
    case sxm::ASTNode_Kind::Num:
        return "Number";
    case sxm::ASTNode_Kind::Ref:
        return "Reference";
    case sxm::ASTNode_Kind::UOp:
        return "Unary Operator";
    case sxm::ASTNode_Kind::BiOp:
        return "Binary Operator";
    case sxm::ASTNode_Kind::Fn:
        return "Function";
    case sxm::ASTNode_Kind::Priority:
        return "Priority";
    }

    return "UNKNOWN ASTNODE";
}

std::string sxm::biop_tostr(sxm::BinaryOp kind) {
    switch (kind) {
    case sxm::BinaryOp::Add: {
        return "+";
    }
    case sxm::BinaryOp::Sub: {
        return "-";
    }
    case sxm::BinaryOp::Div: {
        return "/";
    }
    case sxm::BinaryOp::Mul: {
        return "*";
    }
    case sxm::BinaryOp::Mod: {
        return "%";
    }
    case sxm::BinaryOp::Pow: {
        return "^";
    }
    case sxm::BinaryOp::Assign: {
        return "=";
    }
    case sxm::BinaryOp::Eq: {
        return "==";
    }
    case sxm::BinaryOp::UEq: {
        return "!=";
    }
    case sxm::BinaryOp::GT: {
        return ">";
    }
    case sxm::BinaryOp::GTE: {
        return ">=";
    }
    case sxm::BinaryOp::LT: {
        return "<";
    }
    case sxm::BinaryOp::LTE: {
        return "<=";
    }
    }

    return "UNKNOWN BOPERATOR";
}

std::string sxm::uop_tostr(sxm::UnaryOp kind) {
    switch (kind) {
    case sxm::UnaryOp::Pos: {
        return "+";
    }
    case sxm::UnaryOp::Neg: {
        return "-";
    }
    }

    return "UNKNOWN UOPERATOR";
}
