#include "algo/list/range.hpp"
#include "algo/ptr/rel.hpp"
#include "algo/search_map/h.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/parser/parse.hpp"
#include <expected>
#include <variant>

#pragma once

namespace sxm {

using FEvalError = std::variant<sxm::ParserError, sxm::EvalError>;

struct FEvalNode {
    ASTNode_Kind kind;
    FEvalNode **slot;
    FEvalNode *parent;

    FEvalNode() = default;
    FEvalNode(ASTNode_Kind kind, FEvalNode **slot, FEvalNode *parent)
        : kind(kind), slot(slot), parent(parent) {};

    FEvalNode(const FEvalNode &src) = delete;
    FEvalNode& operator=(const FEvalNode &src) = delete;

    FEvalNode(FEvalNode &&src) = default;
    FEvalNode& operator=(FEvalNode &&src) = default;
};

struct FEvalNodeNum : FEvalNode {
    sxm::NumValue value;

    FEvalNodeNum() = default;
    FEvalNodeNum(FEvalNode **slot, FEvalNode *parent, sxm::NumValue value)
        : FEvalNode(ASTNode_Kind::Num, slot, parent), value(value) {};
};

struct FEvalNodeRef : FEvalNode {
    Range<const char> name;

    FEvalNodeRef(FEvalNode **slot, FEvalNode *parent, Range<const char> name)
        : FEvalNode(ASTNode_Kind::Ref, slot, parent), name(name) {};
};

struct FEvalNodePriority : FEvalNode {
    std::size_t compute_off;
    FEvalNode *child;

    FEvalNodePriority(FEvalNode **slot, FEvalNode *parent, FEvalNode *child)
        : FEvalNode(ASTNode_Kind::Priority, slot, parent), compute_off(0), child(child) {};
};

struct FEvalNodeBiOp : FEvalNode {
    sxm::BinaryOp operation;
    std::size_t compute_off;
    FEvalNode *opr_list[2];

    FEvalNodeBiOp() = default;
    FEvalNodeBiOp(FEvalNode **slot, FEvalNode *parent, BinaryOp operation, FEvalNode *left, FEvalNode *right)
        : FEvalNode(ASTNode_Kind::BiOp, slot, parent), operation(operation), compute_off(0), opr_list { left, right } {};

    inline FEvalNode *left() {
        return this->opr_list[0];
    }

    inline FEvalNode **left_ptr() {
        return this->opr_list;
    }

    inline FEvalNode *right() {
        return this->opr_list[1];
    }

    inline FEvalNode **right_ptr() {
        return this->opr_list + 1;
    }
};

struct FEvalNodeUOp : FEvalNode {
    std::size_t compute_off;
    UnaryOp operation;
    FEvalNode *child;

    FEvalNodeUOp() = default;
    FEvalNodeUOp(FEvalNode **slot, FEvalNode *parent, UnaryOp operation, FEvalNode *child)
        : FEvalNode(ASTNode_Kind::UOp, slot, parent), compute_off(0), operation(operation), child(child) {};
};

struct FEvalNodeFn : FEvalNode {
    Range<const char> name;
    std::size_t compute_off;
    RangeRel<PtrRel<FEvalNode *, 1>> arg_list;

    FEvalNodeFn(FEvalNode **slot, FEvalNode *parent, Range<const char> name, RangeRel<PtrRel<FEvalNode *, 1>> arg_list)
        : FEvalNode(ASTNode_Kind::Fn, slot, parent), name(name), compute_off(0), arg_list(arg_list) {};

    inline void arg_list_push() {
        this->arg_list.end.shift_list(1);
    }

    inline FEvalNode **arg_first(sxm::FEvalNode **memory_start) {
        return this->arg_list.start.ptr_let(memory_start);
    }

    inline FEvalNode **arg_last(sxm::FEvalNode **memory_start) {
        return this->arg_list.end.ptr_let(memory_start) - 1;
    }
};

std::expected<sxm::NumValue, FEvalError> eval_flat(const char *src, HMap<sxm::NumValue> *val_hmap);

}
