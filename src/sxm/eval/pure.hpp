#include "algo/search_map/h.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/parser/parse.hpp"
#include <expected>
#include <vector>

#pragma once

namespace sxm {

struct PEvalNode {
    PEvalNode **slot;
    PEvalNode *parent;
    ASTNode_Kind kind;

    PEvalNode() = default;
    PEvalNode(PEvalNode **slot, PEvalNode *parent, ASTNode_Kind kind)
        : slot(slot), parent(parent), kind(kind) {};

    PEvalNode(const PEvalNode &src) = delete;
    PEvalNode& operator=(const PEvalNode &src) = delete;

    PEvalNode(PEvalNode &&src) = default;
    PEvalNode& operator=(PEvalNode &&src) = default;

    virtual ~PEvalNode() = default;
};

struct PEvalNodeNum : PEvalNode {
    sxm::NumValue value;

    PEvalNodeNum() = default;
    PEvalNodeNum(PEvalNode **slot, PEvalNode *parent, ASTNode_Kind kind, sxm::NumValue value)
        : PEvalNode(slot, parent, kind), value(value) {};

    ~PEvalNodeNum() noexcept = default;
};

struct PEvalNodeRef : PEvalNode {
    Range<const char> name;

    PEvalNodeRef(PEvalNode **slot, PEvalNode *parent, ASTNode_Kind kind, Range<const char> name)
        : PEvalNode(slot, parent, kind), name(name) {};

    ~PEvalNodeRef() noexcept;
};

struct PEvalNodePriority : PEvalNode {
    PEvalNode *child;

    PEvalNodePriority(PEvalNode **slot, PEvalNode *parent, ASTNode_Kind kind, PEvalNode *child)
        : PEvalNode(slot, parent, kind), child(child) {};

    ~PEvalNodePriority() noexcept;
};

struct PEvalNodeBiOp : PEvalNode {
    BinaryOp operation;
    PEvalNode *left;
    PEvalNode *right;

    PEvalNodeBiOp() = default;
    PEvalNodeBiOp(PEvalNode **slot, PEvalNode *parent, ASTNode_Kind kind, BinaryOp operation, PEvalNode *left, PEvalNode *right)
        : PEvalNode(slot, parent, kind), operation(operation), left(left), right(right) {};

    ~PEvalNodeBiOp() noexcept;
};

struct PEvalNodeUOp : PEvalNode {
    UnaryOp operation;
    PEvalNode *child;

    PEvalNodeUOp() = default;
    PEvalNodeUOp(PEvalNode **slot, PEvalNode *parent, ASTNode_Kind kind, UnaryOp operation, PEvalNode *child)
        : PEvalNode(slot, parent, kind), operation(operation), child(child) {};

    ~PEvalNodeUOp() noexcept;
};

struct PEvalNodeFn : PEvalNode {
    Range<const char> name;
    std::vector<PEvalNode *> arg_list;

    PEvalNodeFn(PEvalNode **slot, PEvalNode *parent, ASTNode_Kind kind, Range<const char> name, std::size_t arg_list_size)
        : PEvalNode(slot, parent, kind), name(name), arg_list(arg_list_size) {};

    ~PEvalNodeFn() noexcept;
};

std::expected<sxm::PEvalNode *, sxm::ParserError> eval_pure_build(const char *src);
std::expected<const sxm::NumValue, sxm::EvalError> eval_pure(sxm::PEvalNode *root, HMap<sxm::NumValue> *val_hmap);

}
