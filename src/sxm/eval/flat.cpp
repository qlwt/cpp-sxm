#include "sxm/eval/flat.hpp"
#include "algo/list/range.hpp"
#include "algo/search_map/h.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/ast/fnmap.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/parser/parse.hpp"
#include "sxm/parser/token.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <expected>
#include <utility>

namespace {

constexpr std::size_t nodespace_num = sizeof(sxm::FEvalNodeNum);
constexpr std::size_t nodespace_fn = sizeof(sxm::FEvalNodeFn);
constexpr std::size_t nodespace_ref = sizeof(sxm::FEvalNodeRef);
constexpr std::size_t nodespace_uop = sizeof(sxm::FEvalNodeUOp);
constexpr std::size_t nodespace_biop = sizeof(sxm::FEvalNodeBiOp);
constexpr std::size_t nodespace_priority = sizeof(sxm::FEvalNodePriority);

template <typename T>
inline T *ptrshift(T *ptr, std::size_t shift) {
    return reinterpret_cast<T *>(reinterpret_cast<char *>(ptr) + shift);
}

struct TraverseState {
    sxm::FEvalNode *parent;
    sxm::FEvalNode **treeptr;
};

struct MemoryArg {
    Range<sxm::FEvalNode *> range;
    sxm::FEvalNode **br_now;

    static inline MemoryArg init(std::size_t size) noexcept {
        sxm::FEvalNode **memory_start = reinterpret_cast<sxm::FEvalNode **>(std::malloc(size * sizeof(sxm::FEvalNode *)));

        MemoryArg memory {
            { memory_start, memory_start + size },
            memory_start,
        };

        return memory;
    }

    static inline MemoryArg extend(MemoryArg src, TraverseState *tstate) noexcept {
        std::size_t const dest_size_raw = src.size_raw() * 32;
        std::size_t const dest_size_list = src.size_list() * 32;
        sxm::FEvalNode **const dest_start = reinterpret_cast<sxm::FEvalNode **>(std::realloc(src.range.p_start, dest_size_raw));
        sxm::FEvalNode **const dest_br_now = dest_start + (src.br_now - src.range.p_start);

        if (dest_start != src.range.p_start) {
            for (sxm::FEvalNode **dest_now = dest_start; dest_now < dest_br_now; ++dest_now) {
                (*dest_now)->slot = dest_now;
            }

            if (tstate->treeptr >= src.range.p_start && tstate->treeptr < src.range.p_end) {
                tstate->treeptr = dest_start + (tstate->treeptr - src.range.p_start);
            }
        }

        return {
            .range = { dest_start, dest_start + dest_size_list, },
            .br_now = dest_br_now,
        };
    }

    inline std::size_t size_raw() const noexcept {
        return this->range.size() * sizeof(sxm::FEvalNode *);
    }

    inline std::size_t size_list() const noexcept {
        return this->range.size();
    }

    inline std::size_t size_free_raw() const noexcept {
        return (this->range.p_end - this->br_now) * sizeof(sxm::FEvalNode *);
    }

    inline std::size_t size_free_line() const noexcept {
        return this->range.p_end - this->br_now;
    }

    inline void push() noexcept {
        this->br_now += 1;
    }

    inline void pop() noexcept {
        this->br_now -= 1;
    }

    inline void alloc(std::size_t size) noexcept {
        this->br_now += size;
    }

    inline void free(std::size_t size) noexcept {
        this->br_now -= size;
    }
};

struct MemoryTree {
    static inline MemoryTree init(std::size_t size) {
        char *memory_start = reinterpret_cast<char *>(std::malloc(size));

        MemoryTree memory {
            { memory_start, memory_start + size },
            memory_start + sizeof(sxm::FEvalNode **),
        };

        *memory.root_point() = memory.tree_point_start();

        return memory;
    }

    static MemoryTree memory_extend(MemoryTree src, MemoryArg mem_arg, TraverseState *tstate) {
        const std::size_t dest_size = src.size_raw() * 4;

        char *dest_start = reinterpret_cast<char *>(std::realloc(src.range.p_start, dest_size));

        const long shift = dest_start - src.range.p_start;

        char *dest_br_now = ptrshift(src.br_now, shift);

        if (dest_start != src.range.p_start) {
            tstate->parent = ptrshift(tstate->parent, shift);

            sxm::FEvalNode **root = reinterpret_cast<sxm::FEvalNode **>(dest_start);

            *root = ptrshift(*root, shift);
            (*root)->slot = root;

            if (!(tstate->treeptr >= mem_arg.range.p_start && tstate->treeptr < mem_arg.range.p_end)) {
                tstate->treeptr = ptrshift(tstate->treeptr, shift);
            }

            {
                sxm::FEvalNode *target = *root;

                do {

                    switch (target->kind) {
                    case sxm::ASTNode_Kind::Num: {
                        target = target->parent;

                        break;
                    }
                    case sxm::ASTNode_Kind::Ref: {
                        target = target->parent;

                        break;
                    }
                    case sxm::ASTNode_Kind::Priority: {
                        auto lnode = static_cast<sxm::FEvalNodePriority *>(target);

                        if (lnode->compute_off >= 1) {
                            lnode->compute_off = 0;
                            target = target->parent;
                        } else {
                            lnode->child = ptrshift(lnode->child, shift);

                            if (reinterpret_cast<char *>(lnode->child) < dest_br_now) {
                                lnode->child->parent = target;
                                lnode->child->slot = &lnode->child;

                                target = lnode->child;
                                ++lnode->compute_off;
                            } else {
                                lnode->child->slot = &lnode->child;

                                ++lnode->compute_off;
                            }
                        }

                        break;
                    }
                    case sxm::ASTNode_Kind::UOp: {
                        auto lnode = static_cast<sxm::FEvalNodeUOp *>(target);

                        if (lnode->compute_off >= 1) {
                            lnode->compute_off = 0;
                            target = target->parent;
                        } else {
                            lnode->child = ptrshift(lnode->child, shift);

                            if (reinterpret_cast<char *>(lnode->child) < dest_br_now) {
                                lnode->child->parent = target;
                                lnode->child->slot = &lnode->child;

                                target = lnode->child;
                                ++lnode->compute_off;
                            } else {
                                lnode->child->slot = &lnode->child;

                                ++lnode->compute_off;
                            }
                        }

                        break;
                    }
                    case sxm::ASTNode_Kind::BiOp: {
                        auto lnode = static_cast<sxm::FEvalNodeBiOp *>(target);

                        if (lnode->compute_off >= 2) {
                            lnode->compute_off = 0;
                            target = target->parent;
                        } else {
                            sxm::FEvalNode **operand_slot = lnode->opr_list + lnode->compute_off;
                            sxm::FEvalNode *operand_ptr = ptrshift(*operand_slot, shift);

                            *operand_slot = operand_ptr;

                            if (reinterpret_cast<char *>(operand_ptr) < dest_br_now) {
                                operand_ptr->parent = target;
                                operand_ptr->slot = operand_slot;

                                target = operand_ptr;
                                ++lnode->compute_off;
                            } else {
                                operand_ptr->slot = operand_slot;

                                ++lnode->compute_off;
                            }
                        }


                        break;
                    }
                    case sxm::ASTNode_Kind::Fn: {
                        auto lnode = static_cast<sxm::FEvalNodeFn *>(target);

                        if (lnode->compute_off >= lnode->arg_list.size_list()) {
                            lnode->compute_off = 0;
                            target = target->parent;
                        } else {
                            sxm::FEvalNode **arg = lnode->arg_first(mem_arg.range.p_start) + lnode->compute_off;

                            *arg = ptrshift(*arg, shift);

                            if (reinterpret_cast<char *>(*arg) < dest_br_now) {
                                (*arg)->parent = target;
                                (*arg)->slot = arg;
                                target = *arg;

                                ++lnode->compute_off;
                            } else {
                                (*arg)->slot = arg;
                                ++lnode->compute_off;
                            }
                        }

                        break;
                    }
                    }
                } while (target != nullptr);
            }

            tstate->treeptr = (*tstate->treeptr)->slot;
        }

        return {
            { dest_start, dest_start + dest_size, },
            dest_br_now,
        };
    }

    Range<char> range;
    char *br_now;

    inline std::size_t size_raw() const noexcept {
        return this->range.size();
    }

    inline std::size_t size_free_raw() const noexcept {
        return this->range.p_end - this->br_now;
    }

    inline sxm::FEvalNode **root_point() const noexcept {
        return reinterpret_cast<sxm::FEvalNode **>(this->range.p_start);
    }

    inline sxm::FEvalNodeNum **root_point_num() const noexcept {
        return reinterpret_cast<sxm::FEvalNodeNum **>(this->range.p_start);
    }

    inline sxm::FEvalNode *tree_point_start() const noexcept {
        return reinterpret_cast<sxm::FEvalNode *>(this->root_point() + 1);
    }

    inline sxm::FEvalNode *tree_point_now() const noexcept {
        return reinterpret_cast<sxm::FEvalNode *>(this->br_now);
    }

    inline void tree_alloc_raw(long size) noexcept {
        this->br_now += size;
    }

    inline void tree_setptr(char *target) noexcept {
        this->br_now = target;
    }

    inline void tree_free_raw(long size) noexcept {
        this->br_now -= size;
    }
};

inline sxm::NumValue numvalue_let_biop(sxm::FEvalNodeBiOp *node) {
    const auto lleft = static_cast<sxm::FEvalNodeNum *>(node->left());
    const auto lright = static_cast<sxm::FEvalNodeNum *>(node->right());

    switch (node->operation) {
    case sxm::BinaryOp::Mul: {
        return sxm::NumValue::mul(&lleft->value, &lright->value);
    }
    case sxm::BinaryOp::Div: {
        return sxm::NumValue::div(&lleft->value, &lright->value);
    }
    case sxm::BinaryOp::Sub: {
        return sxm::NumValue::sub(&lleft->value, &lright->value);
    }
    case sxm::BinaryOp::Add: {
        return sxm::NumValue::add(&lleft->value, &lright->value);
    }
    case sxm::BinaryOp::Mod: {
        return sxm::NumValue::mod(&lleft->value, &lright->value);
    }
    case sxm::BinaryOp::Pow: {
        return sxm::NumValue::pow(&lleft->value, &lright->value);
    }
    case sxm::BinaryOp::Eq: {
        return sxm::NumValue::eq(&lleft->value, &lright->value);
    }
    case sxm::BinaryOp::UEq: {
        return sxm::NumValue::ueq(&lleft->value, &lright->value);
    }
    case sxm::BinaryOp::LT: {
        return sxm::NumValue::lt(&lleft->value, &lright->value);
    }
    case sxm::BinaryOp::LTE: {
        return sxm::NumValue::lte(&lleft->value, &lright->value);
    }
    case sxm::BinaryOp::GT: {
        return sxm::NumValue::gt(&lleft->value, &lright->value);
    }
    case sxm::BinaryOp::GTE: {
        return sxm::NumValue::gte(&lleft->value, &lright->value);
    }
    case sxm::BinaryOp::Assign: {
        return lright->value;
    }
    }

    std::unreachable();
}

struct ExprFold_Params {
    MemoryArg mem_arg;
    HMap<sxm::NumValue> *val_hmap;
};

std::expected<sxm::FEvalNode *, sxm::FEvalError> fold_expr(sxm::FEvalNode *target, sxm::FEvalNode *limit, ExprFold_Params params) {
    while (true) {
        switch (target->kind) {
        case sxm::ASTNode_Kind::Num: {
            if (target->parent == limit) {
                return target;
            } else {
                target = target->parent;
            }

            break;
        }
        case sxm::ASTNode_Kind::Ref: {
            const auto lnode = static_cast<sxm::FEvalNodeRef *>(target);
            const std::optional<sxm::NumValue> ref_value = params.val_hmap->find(lnode->name);

            if (!ref_value.has_value()) {
                return std::unexpected {
                    sxm::EvalError {
                        sxm::EvalError_Kind::VarUndef,
                        "Undefined Variable " + std::string{lnode->name.p_start, lnode->name.p_end}
                    },
                };
            }

            {
                sxm::FEvalNode **slot = target->slot;
                sxm::FEvalNode *parent = target->parent;

                new (target) sxm::FEvalNodeNum {
                    slot,
                    parent,
                    ref_value.value(),
                };
            }

            if (target->parent == limit) {
                return target;
            } else {
                target = target->parent;
            }

            break;
        }
        case sxm::ASTNode_Kind::UOp: {
            const auto lnode = static_cast<sxm::FEvalNodeUOp *>(target);

            if (lnode->child->kind != sxm::ASTNode_Kind::Num) {
                target = lnode->child;

                break;
            }

            {
                const auto lchild = static_cast<sxm::FEvalNodeNum *>(lnode->child);

                switch (lnode->operation) {
                case sxm::UnaryOp::Neg: {
                    sxm::FEvalNode **slot = target->slot;
                    sxm::FEvalNode *parent = target->parent;

                    new (target) sxm::FEvalNodeNum {
                        slot,
                        parent,
                        sxm::NumValue::neg(&lchild->value),
                    };

                    break;
                }
                case sxm::UnaryOp::Pos: {
                    sxm::FEvalNode **slot = target->slot;
                    sxm::FEvalNode *parent = target->parent;

                    new (target) sxm::FEvalNodeNum {
                        slot,
                        parent,
                        lchild->value,
                    };

                    break;
                }
                }

                if (target->parent == limit) {
                    return target;
                } else {
                    target = target->parent;

                    break;
                }
            }
        }
        case sxm::ASTNode_Kind::BiOp: {
            const auto lnode = static_cast<sxm::FEvalNodeBiOp *>(target);

            if (lnode->operation == sxm::BinaryOp::Assign && lnode->left()->kind == sxm::ASTNode_Kind::Ref) {
                if (lnode->right()->kind != sxm::ASTNode_Kind::Num) {
                    target = lnode->right();

                    break;
                }

                auto lleft = static_cast<sxm::FEvalNodeRef *>(lnode->left());
                auto lright = static_cast<sxm::FEvalNodeNum *>(lnode->right());

                params.val_hmap->insert(lleft->name, lright->value);

                new (lleft) sxm::FEvalNodeNum {
                    target->slot,
                    target->parent,
                    numvalue_let_biop(lnode),
                };

                *(lleft->slot) = lleft;

                if (lleft->parent == limit) {
                    return lleft;
                } else {
                    target = lleft->parent;
                }

                break;
            }

            {
                if (lnode->left()->kind != sxm::ASTNode_Kind::Num) {
                    target = lnode->left();

                    break;
                }

                if (lnode->right()->kind != sxm::ASTNode_Kind::Num) {
                    target = lnode->right();

                    break;
                }

                {
                    sxm::FEvalNode *lleft = lnode->left();

                    new (lleft) sxm::FEvalNodeNum {
                        target->slot,
                        target->parent,
                        numvalue_let_biop(lnode),
                    };

                    *(lleft->slot) = lleft;

                    if (lleft->parent == limit) {
                        return lleft;
                    } else {
                        target = lleft->parent;

                        break;
                    }
                }
            }
        }
        case sxm::ASTNode_Kind::Priority: {
            const auto lnode = static_cast<sxm::FEvalNodePriority *>(target);

            return lnode->child;
        }
        case sxm::ASTNode_Kind::Fn: {
            const auto lnode = static_cast<sxm::FEvalNodeFn *>(target);

            return *lnode->arg_last(params.mem_arg.range.p_start);
        }
        default:
            std::unreachable();
        }
    };
}

}

std::expected<sxm::NumValue, sxm::FEvalError> sxm::eval_flat(const char *src, HMap<sxm::NumValue> *val_hmap) {
    MemoryArg mem_arg = MemoryArg::init(32);
    MemoryTree mem_tree = MemoryTree::init(1 << 10);

    TraverseState tstate {
        .parent = nullptr,
        .treeptr = mem_tree.root_point(),
    };

    auto token_gen = sxm::parse(src);
    auto token_gen_now = token_gen.begin();

    while (true) {
        std::expected<sxm::Token, sxm::ParserError> &token_now = *token_gen_now;

        if (!token_now.has_value()) {
            std::free(mem_arg.range.p_start);
            std::free(mem_tree.range.p_start);

            return std::unexpected { token_now.error() };
        }

        switch (token_now->kind) {
        case sxm::Token_Kind::FnOpen: {
            std::unreachable();
        }
        case sxm::Token_Kind::Space: {
            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::End: {
            auto eval_report = fold_expr(*mem_tree.root_point(), nullptr, {
                .mem_arg = mem_arg,
                .val_hmap = val_hmap,
            });

            if (!eval_report.has_value()) {
                std::free(mem_arg.range.p_start);
                std::free(mem_tree.range.p_start);

                return std::unexpected { eval_report.error() };
            }

            sxm::NumValue result = (*mem_tree.root_point_num())->value;

            std::free(mem_tree.range.p_start);
            std::free(mem_arg.range.p_start);

            return result;
        }
        case sxm::Token_Kind::NumberInt: {
            if (mem_tree.size_free_raw() < nodespace_num) {
                mem_tree = MemoryTree::memory_extend(mem_tree, mem_arg, &tstate);
            }

            new (*tstate.treeptr) sxm::FEvalNodeNum {
                tstate.treeptr,
                tstate.parent,
                sxm::NumValue::parse_int(token_now->section_start, token_now->section_end),
            };

            mem_tree.tree_alloc_raw(nodespace_num);
            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::NumberReal: {
            if (mem_tree.size_free_raw() < nodespace_num) {
                mem_tree = MemoryTree::memory_extend(mem_tree, mem_arg, &tstate);
            }

            new (*tstate.treeptr) sxm::FEvalNodeNum {
                tstate.treeptr,
                tstate.parent,
                sxm::NumValue::parse_real(token_now->section_start, token_now->section_end),
            };

            mem_tree.tree_alloc_raw(nodespace_num);

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::Reference: {
            Range<const char> name { token_now->section_start, token_now->section_end, };

            ++token_gen_now;

            if (!token_now.has_value()) {
                std::free(mem_arg.range.p_start);
                std::free(mem_tree.range.p_start);

                return std::unexpected { token_now.error() };
            }

            if (token_now->kind == sxm::Token_Kind::FnOpen) {
                ++token_gen_now;

                if (!token_now.has_value()) {
                    return std::unexpected { token_now.error() };
                }

                if (token_now->kind == sxm::Token_Kind::FnClose) {
                    if (mem_tree.size_free_raw() < nodespace_num) {
                        mem_tree = MemoryTree::memory_extend(mem_tree, mem_arg, &tstate);
                    }

                    std::optional<sxm::NumFnFEval> lnode_fn = sxm::fn_hmap_feval.find(name);

                    if (!lnode_fn.has_value()) {
                        std::free(mem_arg.range.p_start);
                        std::free(mem_tree.range.p_start);

                        return std::unexpected {
                            sxm::EvalError {
                                sxm::EvalError_Kind::FnUndef,
                                "Undefined Function " + std::string { name.p_start, name.p_end, }
                            }
                        };
                    }

                    std::optional<sxm::NumValue> lnode_result = lnode_fn.value()({});

                    if (!lnode_result.has_value()) {
                        std::free(mem_arg.range.p_start);
                        std::free(mem_tree.range.p_start);

                        return std::unexpected {
                            sxm::EvalError {
                                sxm::EvalError_Kind::FnEval,
                                "Failed to evaluate function " + std::string { name.p_start, name.p_end, }
                            }
                        };
                    }

                    new (*tstate.treeptr) sxm::FEvalNodeNum {
                        tstate.treeptr,
                        tstate.parent,
                        lnode_result.value()
                    };

                    mem_tree.tree_alloc_raw(nodespace_num);

                    ++token_gen_now;
                } else {
                    if (mem_tree.size_free_raw() < nodespace_fn) {
                        mem_tree = MemoryTree::memory_extend(mem_tree, mem_arg, &tstate);
                    }

                    if (mem_arg.size_free_line() < 1) {
                        mem_arg = MemoryArg::extend(mem_arg, &tstate);
                    }

                    auto lnode = new (*tstate.treeptr) sxm::FEvalNodeFn {
                        tstate.treeptr,
                        tstate.parent,
                        { name.p_start, name.p_end, },
                        {
                            { (mem_arg.br_now - mem_arg.range.p_start) * (long)sizeof(sxm::FEvalNode **) },
                            { (mem_arg.br_now - mem_arg.range.p_start + 1) * (long)sizeof(sxm::FEvalNode **) },
                        },
                    };

                    tstate.parent = lnode;
                    mem_tree.tree_alloc_raw(nodespace_fn);

                    *lnode->arg_first(mem_arg.range.p_start) = mem_tree.tree_point_now();

                    mem_arg.push();
                    tstate.treeptr = lnode->arg_first(mem_arg.range.p_start);
                }
            } else {
                if (mem_tree.size_free_raw() < nodespace_ref) {
                    mem_tree = MemoryTree::memory_extend(mem_tree, mem_arg, &tstate);
                }

                new (*tstate.treeptr) sxm::FEvalNodeRef {
                    tstate.treeptr,
                    tstate.parent,
                    { name.p_start, name.p_end, }
                };

                mem_tree.tree_alloc_raw(nodespace_ref);
            }

            break;
        }
        case sxm::Token_Kind::FnComma: {
            if (mem_tree.size_free_raw() < nodespace_num) {
                mem_tree = MemoryTree::memory_extend(mem_tree, mem_arg, &tstate);
            }

            if (mem_arg.size_free_line() < 1) {
                mem_arg = MemoryArg::extend(mem_arg, &tstate);
            }

            sxm::FEvalNode *target = (*tstate.treeptr)->parent;

            while (target->kind != sxm::ASTNode_Kind::Fn) {
                target = target->parent;
            }

            {
                auto ltarget = static_cast<sxm::FEvalNodeFn *>(target);

                auto eval_report = fold_expr(*ltarget->arg_last(mem_arg.range.p_start), nullptr, {
                    .mem_arg = mem_arg,
                    .val_hmap =val_hmap
                });

                if (!eval_report.has_value()) {
                    std::free(mem_arg.range.p_start);
                    std::free(mem_tree.range.p_start);

                    return std::unexpected { eval_report.error() };
                }

                mem_arg.push();
                mem_tree.tree_setptr(reinterpret_cast<char *>(eval_report.value()) + nodespace_num);

                ltarget->arg_list_push();

                tstate.parent = target;
                tstate.treeptr = ltarget->arg_last(mem_arg.range.p_start);
                *ltarget->arg_last(mem_arg.range.p_start) = mem_tree.tree_point_now();
            }

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::FnClose: {
            sxm::FEvalNode *target = (*tstate.treeptr)->parent;

            while (target->kind != sxm::ASTNode_Kind::Fn) {
                target = target->parent;
            }

            tstate.treeptr = target->slot;
            tstate.parent = target->parent;

            {
                auto ltarget = static_cast<sxm::FEvalNodeFn *>(target);
                Range<const char> name = ltarget->name;

                auto eval_report = fold_expr(*ltarget->arg_last(mem_arg.range.p_start), nullptr, {
                    .mem_arg = mem_arg,
                    .val_hmap = val_hmap
                });

                if (!eval_report.has_value()) {
                    std::free(mem_arg.range.p_start);
                    std::free(mem_tree.range.p_start);

                    return std::unexpected { eval_report.error() };
                }

                std::optional<sxm::NumFnFEval> ltarget_fn = sxm::fn_hmap_feval.find(ltarget->name);

                if (!ltarget_fn.has_value()) {
                    std::free(mem_arg.range.p_start);
                    std::free(mem_tree.range.p_start);

                    return std::unexpected {
                        sxm::EvalError {
                            sxm::EvalError_Kind::FnUndef,
                            "Could not find Function " + std::string { name.p_start, name.p_end }
                        }
                    };
                }

                std::optional<sxm::NumValue> ltarget_result = ltarget_fn.value()({
                    reinterpret_cast<sxm::FEvalNodeNum **>(ltarget->arg_first(mem_arg.range.p_start)),
                    reinterpret_cast<sxm::FEvalNodeNum **>(ltarget->arg_last(mem_arg.range.p_start)) + 1,
                });

                if (!ltarget_result.has_value()) {
                    std::free(mem_arg.range.p_start);
                    std::free(mem_tree.range.p_start);

                    return std::unexpected {
                        sxm::EvalError {
                            sxm::EvalError_Kind::FnEval,
                            "Failed to evaluate function " + std::string { name.p_start, name.p_end }
                        }
                    };
                }

                mem_arg.free(ltarget->arg_list.size_list());

                {
                    sxm::FEvalNode **slot = ltarget->slot;
                    sxm::FEvalNode *parent = ltarget->parent;

                    new (ltarget) sxm::FEvalNodeNum {
                        slot,
                        parent,
                        ltarget_result.value(),
                    };
                }

                mem_tree.tree_setptr(reinterpret_cast<char *>(target) + nodespace_num);
            }

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::PriorityOpen: {
            if (mem_tree.size_free_raw() < nodespace_priority) {
                mem_tree = MemoryTree::memory_extend(mem_tree, mem_arg, &tstate);
            }

            mem_tree.tree_alloc_raw(nodespace_priority);

            auto node_priority = new (*tstate.treeptr) sxm::FEvalNodePriority {
                tstate.treeptr,
                tstate.parent,
                mem_tree.tree_point_now(),
            };

            {
                tstate.parent = node_priority;
                tstate.treeptr = &node_priority->child;
            }

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::PriorityClose: {
            sxm::FEvalNode *target = (*tstate.treeptr)->parent;

            while (target->kind != sxm::ASTNode_Kind::Priority) {
                target = target->parent;
            }

            auto lnode = static_cast<sxm::FEvalNodePriority *>(target);
            auto eval_report = fold_expr(lnode->child, nullptr, {
                .mem_arg = mem_arg,
                .val_hmap = val_hmap
            });

            if (!eval_report.has_value()) {
                std::free(mem_arg.range.p_start);
                std::free(mem_tree.range.p_start);

                return std::unexpected { eval_report.error() };
            }

            tstate.treeptr = target->slot;
            tstate.parent = target->parent;

            {
                sxm::FEvalNode **slot = target->slot;
                sxm::FEvalNode *parent = target->parent;
                sxm::FEvalNodeNum *child = static_cast<sxm::FEvalNodeNum *>(lnode->child);

                new (target) sxm::FEvalNodeNum {
                    slot,
                    parent,
                    child->value
                };
            }

            mem_tree.tree_setptr(reinterpret_cast<char *>(target) + nodespace_num);

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::UnaryOp: {
            if (mem_tree.size_free_raw() < nodespace_uop) {
                mem_tree = MemoryTree::memory_extend(mem_tree, mem_arg, &tstate);
            }

            const sxm::UnaryOp uop = sxm::uop_let(token_now->section_start);

            auto node_uop = new (*tstate.treeptr) sxm::FEvalNodeUOp {
                tstate.treeptr,
                tstate.parent,
                uop,
                reinterpret_cast<sxm::FEvalNode *>(mem_tree.br_now + nodespace_uop),
            };

            {
                tstate.parent = node_uop;
                tstate.treeptr = &node_uop->child;
            }

            mem_tree.tree_alloc_raw(nodespace_uop);

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::BinaryOp: {
            if (mem_tree.size_free_raw() < nodespace_biop) {
                mem_tree = MemoryTree::memory_extend(mem_tree, mem_arg, &tstate);
            }

            const sxm::BinaryOp biop = sxm::biop_let(token_now->section_start, token_now->section_end);
            const unsigned short biop_priority = sxm::biop_priority_let(biop);

            auto target = *tstate.treeptr;
            while (target->parent != nullptr) {
                if (target->parent->kind == sxm::ASTNode_Kind::UOp) {
                    target = target->parent;

                    continue;
                } else if (target->parent->kind == sxm::ASTNode_Kind::BiOp) {
                    const auto target_parent = static_cast<sxm::FEvalNodeBiOp *>(target->parent);
                    const unsigned short target_parent_priority = sxm::biop_priority_let(target_parent->operation);

                    if (target_parent_priority >= biop_priority) {
                        target = target_parent;

                        continue;
                    } else {
                        break;
                    }
                }

                break;
            }

            if (!(biop == sxm::BinaryOp::Assign && target->kind == sxm::ASTNode_Kind::Ref)) {
                std::expected<sxm::FEvalNode *, sxm::FEvalError> eval_report;

                eval_report = fold_expr(target, target->parent, {
                    .mem_arg = mem_arg,
                    .val_hmap = val_hmap
                });

                if (!eval_report.has_value()) {
                    std::free(mem_arg.range.p_start);
                    std::free(mem_tree.range.p_start);

                    return std::unexpected { eval_report.error() };
                }

                target = eval_report.value();
                mem_tree.tree_setptr(reinterpret_cast<char *>(target) + nodespace_num);
            }

            auto node_biop = new (mem_tree.br_now) sxm::FEvalNodeBiOp {
                target->slot,
                target->parent,
                biop,
                target,
                reinterpret_cast<sxm::FEvalNode *>(mem_tree.br_now + nodespace_biop),
            };

            {
                *(node_biop->slot) = node_biop;
                target->parent = node_biop;
                target->slot = node_biop->left_ptr();
            }

            {
                tstate.parent = node_biop;
                tstate.treeptr = node_biop->right_ptr();
            }

            mem_tree.tree_alloc_raw(nodespace_biop);

            ++token_gen_now;
            break;
        }
        }
    }
}
