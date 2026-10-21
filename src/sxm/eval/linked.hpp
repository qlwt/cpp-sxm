#include "algo/search_map/h.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/parser/parse.hpp"
#include <expected>
#include <variant>
#include <vector>

#pragma once

namespace sxm {

using LEvalError = std::variant<sxm::ParserError, sxm::EvalError>;

struct LEvalNode {
    ASTNode_Kind kind;
    LEvalNode **slot;
    LEvalNode *parent;

    LEvalNode() = default;
    LEvalNode(ASTNode_Kind kind, LEvalNode **slot, LEvalNode *parent)
        : kind(kind), slot(slot), parent(parent) {};

    LEvalNode(const LEvalNode &src) = delete;
    LEvalNode& operator=(const LEvalNode &src) = delete;

    LEvalNode(LEvalNode &&src) = default;
    LEvalNode& operator=(LEvalNode &&src) = default;

    virtual ~LEvalNode() = default;
};

struct LEvalNodeNum : LEvalNode {
    sxm::NumValue value;

    LEvalNodeNum() = default;
    LEvalNodeNum(LEvalNode **slot, LEvalNode *parent, sxm::NumValue value)
        : LEvalNode(ASTNode_Kind::Num, slot, parent), value(value) {};

    ~LEvalNodeNum() = default;
};

struct LEvalNodeRef : LEvalNode {
    Range<const char> name;

    LEvalNodeRef(LEvalNode **slot, LEvalNode *parent, Range<const char> name)
        : LEvalNode(ASTNode_Kind::Ref, slot, parent), name(name) {};

    ~LEvalNodeRef() = default;
};

struct LEvalNodePriority : LEvalNode {
    LEvalNode *child;

    LEvalNodePriority(LEvalNode **slot, LEvalNode *parent, LEvalNode *child)
        : LEvalNode(ASTNode_Kind::Priority, slot, parent), child(child) {};

    ~LEvalNodePriority();
};

struct LEvalNodeBiOp : LEvalNode {
    sxm::BinaryOp operation;
    LEvalNode *left;
    LEvalNode *right;

    LEvalNodeBiOp() = default;
    LEvalNodeBiOp(LEvalNode **slot, LEvalNode *parent, BinaryOp operation, LEvalNode *left, LEvalNode *right)
        : LEvalNode(ASTNode_Kind::BiOp, slot, parent), operation(operation), left(left), right(right) {};

    ~LEvalNodeBiOp();
};

struct LEvalNodeUOp : LEvalNode {
    UnaryOp operation;
    LEvalNode *child;

    LEvalNodeUOp() = default;
    LEvalNodeUOp(LEvalNode **slot, LEvalNode *parent, UnaryOp operation, LEvalNode *child)
        : LEvalNode(ASTNode_Kind::UOp, slot, parent), operation(operation), child(child) {};

    ~LEvalNodeUOp();
};

struct LEvalNodeFn : LEvalNode {
    Range<const char> name;
    std::vector<LEvalNode *> arg_list;

    LEvalNodeFn(LEvalNode **slot, LEvalNode *parent, Range<const char> name, std::vector<LEvalNode *> arg_list)
        : LEvalNode(ASTNode_Kind::Fn, slot, parent), name(name), arg_list(arg_list) {};

    ~LEvalNodeFn();
};

std::expected<sxm::NumValue, LEvalError> eval_linked(const char *src, HMap<sxm::NumValue> *val_hmap);

}
