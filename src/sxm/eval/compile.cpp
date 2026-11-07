#include "sxm/eval/compile.hpp"
#include "algo/list/range.hpp"
#include "algo/search_map/h_fallback.hpp"
#include "algo/search_map/h_static.hpp"
#include "algo/search_map/i.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/parser/parse.hpp"
#include "sxm/parser/token.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <expected>
#include <utility>

sxm::CEvalNodeUOp::~CEvalNodeUOp() {
    delete this->child;
};

sxm::CEvalNodeBiOp::~CEvalNodeBiOp() {
    delete this->left;
    delete this->right;
};

sxm::CEvalNodePriority::~CEvalNodePriority() {
    delete this->child;
};

sxm::CEvalNodeFn::~CEvalNodeFn() {
    for (auto arg : this->arg_list) {
        delete arg;
    }
};

namespace {

bool uop_status_cexpr(sxm::CEvalNodeUOp *target) noexcept {
    return static_cast<sxm::CEvalNodeNum *>(target->child)->offset.flag_const();
}

bool biop_status_cexpr(sxm::CEvalNodeBiOp *target) noexcept {
    bool left_cexpr = static_cast<sxm::CEvalNodeNum *>(target->left)->offset.flag_const();
    bool right_cexpr = static_cast<sxm::CEvalNodeNum *>(target->right)->offset.flag_const();

    return left_cexpr && right_cexpr;
}

bool fn_status_cexpr(sxm::CEvalNodeFn *target) noexcept {
    for (sxm::CEvalNode *arg : target->arg_list) {
        if (!static_cast<sxm::CEvalNodeNum *>(arg)->offset.flag_const()) {
            return false;
        }
    }

    return true;
}

template <typename T>
struct MicroVector {
    T *start;
    T *now;
    T *end;

    MicroVector(std::size_t size, std::size_t offset) {
        this->start = reinterpret_cast<T *>(std::malloc(size * sizeof(T)));
        this->now = this->start + offset;
        this->end = this->start + size;
    }

    inline std::size_t size() {
        return this->now - this->start;
    }

    inline std::size_t capacity() {
        return this->end - this->start;
    }

    inline void extend() {
        std::size_t next_capacity = (this->capacity() * 2) + (this->capacity() == 0);

        T *next_start = reinterpret_cast<T *>(std::realloc(this->start, next_capacity * sizeof(T)));

        if (next_start != this->start) {
            this->now = next_start + (this->now - this->start);
            this->start = next_start;
        }

        this->end = next_start + next_capacity;
    }

    inline void fit() {
        std::size_t next_capacity = this->size();

        T *next_start = reinterpret_cast<T *>(std::realloc(this->start, next_capacity * sizeof(T)));

        if (next_start != this->start) {
            this->now = next_start + (this->now - this->start);
            this->start = next_start;
        }

        this->end = next_start + next_capacity;
    }

    inline void set(sxm::CEvalOffset offset, const T &value) {
        *(this->start + offset.get()) = value;
    }

    inline std::size_t push(const T &value) {
        *this->now = value;

        ++this->now;

        if (this->now >= this->end) {
            this->extend();
        }

        return this->now - this->start - 1;
    }

    inline void shrink(std::size_t size) {
        this->now -= size;
    }

    inline void destroy() {
        std::free(this->start);
    }
};

struct ArgTrack {
    std::size_t now { 0 };
    std::size_t max { 0 };

    inline std::size_t push() {
        return this->now++;
    }

    inline void take(std::size_t diff) {
        this->max = std::max(this->now, this->max);
        this->now -= diff;
    }

    inline void take_silent(std::size_t diff) {
        this->now -= diff;
    }
};

struct StateTrack {
    std::size_t now { 0 };
    std::vector<sxm::CEvalOffset> hole_list {};

    inline sxm::CEvalOffset alloc() {
        if (this->hole_list.size() >= 1) {
            sxm::CEvalOffset hole = this->hole_list.back();

            this->hole_list.pop_back();

            return hole;
        }

        return { this->now++, false };
    }

    inline void free(sxm::CEvalOffset offset) {
        if (offset.flag_state()) {
            this->hole_list.push_back(offset);
        }
    }

    inline void free_num(sxm::CEvalNodeNum *num) {
        if (num->offset.flag_state() && !num->flag_persistent) {
            this->free(num->offset);
        }
    }
};

struct ConstTrack {
    MicroVector<sxm::NumValue> mvector;
    std::vector<sxm::CEvalOffset> hole_list;

    ConstTrack(std::size_t size, std::size_t offset)
        : mvector(size, offset), hole_list {} {}

    inline sxm::NumValue at(sxm::CEvalOffset offset) {
        return *(this->mvector.start + offset.get());
    }

    inline sxm::CEvalOffset push(sxm::NumValue value) {
        if (this->hole_list.size() >= 1) {
            sxm::CEvalOffset hole = this->hole_list.back();

            this->hole_list.pop_back();
            this->mvector.set(hole, value);

            return hole;
        }

        return { this->mvector.push(value), true };
    }

    inline void free(sxm::CEvalOffset offset) {
        if (offset.flag_const()) {
            this->hole_list.push_back(offset);
        }
    }

    inline void free_num(sxm::CEvalNodeNum *num) {
        if (num->offset.flag_const() && !num->flag_persistent) {
            this->free(num->offset);
        }
    }

    inline void destroy() {
        this->mvector.destroy();
    }
};

struct TraversalState {
    sxm::CEvalNode *parent;
    sxm::CEvalNode **treeptr;

    ArgTrack arg_track;

    ConstTrack const_track;

    StateTrack state_track;
    std::size_t state_rsize;

    sxm::CEvalOffset output;

    MicroVector<sxm::CEvalInstr> inst_list;

    IMap<sxm::NumValue> *sxmvar_const;
    HMapFallback<sxm::CEvalOffset> sxmvar_local;

    // deletes things that are not cleared on destructor
    void destroy() {
        this->const_track.destroy();
        this->inst_list.destroy();

        delete *this->treeptr;
    }
};

inline sxm::CEvalInstr_Kind instruction_kind_let_uop(sxm::UnaryOp op) {
    switch (op) {
    case sxm::UnaryOp::Neg: {
        return sxm::CEvalInstr_Kind::Neg;
    }
    case sxm::UnaryOp::Pos: {
        return sxm::CEvalInstr_Kind::Set;
    }
    default: {
        std::unreachable();
    }
    }
}

inline sxm::CEvalInstr_Kind instruction_kind_let_biop(sxm::BinaryOp op) {
    switch (op) {
    case sxm::BinaryOp::Add: {
        return sxm::CEvalInstr_Kind::Add;
    }
    case sxm::BinaryOp::Sub: {
        return sxm::CEvalInstr_Kind::Sub;
    }
    case sxm::BinaryOp::Div: {
        return sxm::CEvalInstr_Kind::Div;
    }
    case sxm::BinaryOp::Mul: {
        return sxm::CEvalInstr_Kind::Mul;
    }
    case sxm::BinaryOp::Mod: {
        return sxm::CEvalInstr_Kind::Mod;
    }
    case sxm::BinaryOp::Pow: {
        return sxm::CEvalInstr_Kind::Pow;
    }
    case sxm::BinaryOp::Assign: {
        return sxm::CEvalInstr_Kind::Assign;
    }
    case sxm::BinaryOp::Eq: {
        return sxm::CEvalInstr_Kind::Eq;
    }
    case sxm::BinaryOp::UEq: {
        return sxm::CEvalInstr_Kind::UEq;
    }
    case sxm::BinaryOp::GT: {
        return sxm::CEvalInstr_Kind::GT;
    }
    case sxm::BinaryOp::GTE: {
        return sxm::CEvalInstr_Kind::GTE;
    }
    case sxm::BinaryOp::LT: {
        return sxm::CEvalInstr_Kind::LT;
    }
    case sxm::BinaryOp::LTE: {
        return sxm::CEvalInstr_Kind::LTE;
    }
    default: {
        std::unreachable();
    }
    }
}

std::optional<sxm::CEvalError> fold_expr(sxm::CEvalNode *target, sxm::CEvalNode *limit, TraversalState &tstate) {
    do {
        switch (target->kind) {
        case sxm::ASTNode_Kind::Fn: {
            return std::nullopt;
        }
        case sxm::ASTNode_Kind::Num: {
            target = target->parent;

            break;
        }
        case sxm::ASTNode_Kind::Ref: {
            auto lnode = static_cast<sxm::CEvalNodeRef *>(target);

            std::optional<sxm::CEvalOffset> ref_offset = tstate.sxmvar_local.find(lnode->name);

            if (!ref_offset.has_value()) {
                std::optional<sxm::NumValue> ref_value = tstate.sxmvar_const->find(lnode->name);

                if (ref_value.has_value()) {
                    ref_offset = tstate.const_track.push(ref_value.value());
                } else {
                    return sxm::EvalError {
                        sxm::EvalError_Kind::VarUndef,
                        "Undefined Variable " + std::string{lnode->name.p_start, lnode->name.p_end}
                    };
                }
            }

            sxm::CEvalNode **slot = target->slot;
            sxm::CEvalNode *parent = target->parent;
            sxm::CEvalOffset res_offset = ref_offset.value();

            *slot = new sxm::CEvalNodeNum {
                slot,
                parent,
                res_offset,
                true
            };

            tstate.output = res_offset;

            delete lnode;
            target = parent;
            break;
        }
        case sxm::ASTNode_Kind::UOp: {
            auto lnode = static_cast<sxm::CEvalNodeUOp *>(target);

            if (lnode->child->kind != sxm::ASTNode_Kind::Num) {
                target = lnode->child;

                break;
            }

            sxm::CEvalNode **slot = lnode->slot;
            sxm::CEvalNode *parent = lnode->parent;
            sxm::CEvalNodeNum *child = static_cast<sxm::CEvalNodeNum *>(lnode->child);

            if (uop_status_cexpr(lnode)) {
                sxm::NumValue *child_value = tstate.const_track.mvector.start + child->offset.get();
                sxm::NumValue res_value = sxm::uop_apply(lnode->operation, *child_value);

                sxm::CEvalOffset res_offset = tstate.const_track.push(res_value);

                *slot = new sxm::CEvalNodeNum {
                    slot,
                    parent,
                    res_offset,
                    false
                };

                tstate.output = res_offset;
            } else {
                tstate.state_track.free_num(child);

                sxm::CEvalOffset res_offset = tstate.state_track.alloc();

                tstate.inst_list.push({
                    .kind = instruction_kind_let_uop(lnode->operation),
                    .left { .offset { child->offset } },
                    .right {},
                    .output { .offset { res_offset } },
                });

                *slot = new sxm::CEvalNodeNum {
                    slot,
                    parent,
                    res_offset,
                    false
                };

                tstate.output = res_offset;
            }

            delete lnode;
            target = parent;
            break;
        }
        case sxm::ASTNode_Kind::BiOp: {
            auto lnode = static_cast<sxm::CEvalNodeBiOp *>(target);

            if (lnode->operation == sxm::BinaryOp::Assign && lnode->left->kind == sxm::ASTNode_Kind::Ref) {
                if (lnode->right->kind != sxm::ASTNode_Kind::Num) {
                    target = lnode->right;

                    break;
                }

                auto lleft = static_cast<sxm::CEvalNodeRef *>(lnode->left);
                auto lright = static_cast<sxm::CEvalNodeNum *>(lnode->right);
                std::optional<sxm::CEvalOffset> var_offset = tstate.sxmvar_local.find(lleft->name);

                sxm::CEvalNode **slot = lnode->slot;
                sxm::CEvalNode *parent = lnode->parent;

                // if state is one of reserved variables - it must preserve the slot
                if (
                    lright->offset.flag_const()
                    && !(
                        var_offset.has_value()
                        && var_offset->flag_state()
                        && var_offset->get() < tstate.state_rsize
                    )
                ) {
                    if (var_offset.has_value()) {
                        tstate.const_track.free(*var_offset);
                    }

                    tstate.const_track.free_num(lright);
                    sxm::CEvalOffset ref_offset_next = tstate.const_track.push(tstate.const_track.at(lright->offset));

                    tstate.sxmvar_local.insert(lleft->name, ref_offset_next);

                    *slot = new sxm::CEvalNodeNum {
                        slot,
                        parent,
                        ref_offset_next,
                        true,
                    };

                    tstate.output = ref_offset_next;
                } else {
                    tstate.state_track.free_num(lright);

                    if (!var_offset.has_value() || var_offset->flag_const()) {
                        var_offset = tstate.state_track.alloc();

                        tstate.sxmvar_local.insert(lleft->name, var_offset.value());
                    }

                    tstate.inst_list.push({
                        .kind = sxm::CEvalInstr_Kind::Assign,
                        .left {},
                        .right { .offset { lright->offset } },
                        .output { .offset { var_offset.value() } },
                    });

                    *slot = new sxm::CEvalNodeNum {
                        slot,
                        parent,
                        *var_offset,
                        true,
                    };

                    tstate.output = var_offset.value();
                }

                delete lnode;
                target = parent;
            } else {
                if (lnode->left->kind != sxm::ASTNode_Kind::Num) {
                    target = lnode->left;

                    break;
                }

                if (lnode->right->kind != sxm::ASTNode_Kind::Num) {
                    target = lnode->right;

                    break;
                }

                sxm::CEvalNode **slot = lnode->slot;
                sxm::CEvalNode *parent = lnode->parent;
                auto lleft = static_cast<sxm::CEvalNodeNum *>(lnode->left);
                auto lright = static_cast<sxm::CEvalNodeNum *>(lnode->right);

                if (biop_status_cexpr(lnode)) {
                    sxm::NumValue *lleft_value = tstate.const_track.mvector.start + lleft->offset.get();
                    sxm::NumValue *lright_value = tstate.const_track.mvector.start + lright->offset.get();
                    sxm::NumValue res_value = sxm::biop_apply(lnode->operation, *lleft_value, *lright_value);

                    tstate.const_track.free_num(lleft);
                    tstate.const_track.free_num(lright);

                    sxm::CEvalOffset res_offset = tstate.const_track.push(res_value);

                    *slot = new sxm::CEvalNodeNum {
                        slot,
                        parent,
                        res_offset,
                        false
                    };

                    tstate.output = res_offset;
                } else {
                    tstate.state_track.free_num(lleft);
                    tstate.state_track.free_num(lright);

                    sxm::CEvalOffset res_offset = tstate.state_track.alloc();

                    tstate.inst_list.push({
                        .kind = instruction_kind_let_biop(lnode->operation),
                        .left { .offset { lleft->offset } },
                        .right { .offset { lright->offset } },
                        .output { .offset { res_offset } },
                    });

                    *slot = new sxm::CEvalNodeNum {
                        slot,
                        parent,
                        res_offset,
                        false
                    };

                    tstate.output = res_offset;
                }

                delete lnode;
                target = parent;
            }

            break;
        }
        case sxm::ASTNode_Kind::Priority: {
            auto lnode = static_cast<sxm::CEvalNodePriority *>(target);

            if (lnode->child->kind != sxm::ASTNode_Kind::Num) {
                target = lnode->child;

                break;
            }

            auto lchild = static_cast<sxm::CEvalNodeNum *>(lnode->child);

            sxm::CEvalNode **slot = lnode->slot;
            sxm::CEvalNode *parent = lnode->parent;
            sxm::CEvalOffset child_offset = lchild->offset;
            bool child_flag_persistent = lchild->flag_persistent;

            *slot = new sxm::CEvalNodeNum {
                slot,
                parent,
                child_offset,
                child_flag_persistent
            };

            tstate.output = child_offset;

            delete lnode;
            target = parent;
            break;
        }
        }
    } while (target != limit);

    return std::nullopt;
}

}

std::expected<sxm::CEvalDeclaration, sxm::CEvalError> sxm::eval_compile_build(const char *src, const CEval_CompileBuild_Config &config) {
    sxm::CEvalNode *root;
    HMapStatic<32, sxm::CEvalOffset> sxmvar_local;

    TraversalState tstate {
        .parent = nullptr,
        .treeptr = &root,

        .arg_track {},

        .const_track { 16, 0 },

        .state_track { .now = config.sxmvar_state_size },
        .state_rsize = config.sxmvar_state_size,

        .output {},

        .inst_list { 16, 0 },

        .sxmvar_const = config.sxmvar_const,
        .sxmvar_local = HMapFallback<sxm::CEvalOffset> { config.sxmvar_state, &sxmvar_local, },
    };

    auto token_gen = sxm::parse(src);
    auto token_it = token_gen.begin();

    while (true) {
        std::expected<sxm::Token, sxm::ParserError> &token = *token_it;

        if (!token.has_value()) {
            tstate.destroy();

            return std::unexpected { token.error(), };
        }

        switch (token->kind) {
        case sxm::Token_Kind::Space:
        case sxm::Token_Kind::FnOpen:
            std::unreachable();
        case sxm::Token_Kind::End: {
            std::optional<sxm::LEvalError> fold_report = fold_expr(root, nullptr, tstate);

            if (fold_report.has_value()) {
                tstate.destroy();

                return std::unexpected { fold_report.value(), };
            }

            tstate.inst_list.push({
                .kind = sxm::CEvalInstr_Kind::Ret,
                .left { .offset = tstate.output },
                .right {},
                .output {},
            });

            tstate.inst_list.fit();
            tstate.const_track.mvector.fit();

            delete root;

            return sxm::CEvalDeclaration {
                tstate.arg_track.max,
                tstate.state_track.now,
                { tstate.const_track.mvector.start, tstate.const_track.mvector.end },
                { tstate.inst_list.start, tstate.inst_list.end },
            };
        }
        case sxm::Token_Kind::NumberInt: {
            sxm::NumValue res_value = sxm::NumValue::parse_int(token->section_start, token->section_end);
            sxm::CEvalOffset res_offset = tstate.const_track.push(res_value);

            *tstate.treeptr = new CEvalNodeNum {
                tstate.treeptr,
                tstate.parent,
                res_offset,
                false,
            };

            tstate.output = res_offset;

            ++token_it;
            break;
        }
        case sxm::Token_Kind::NumberReal: {
            sxm::NumValue res_value = sxm::NumValue::parse_real(token->section_start, token->section_end);
            sxm::CEvalOffset res_offset = tstate.const_track.push(res_value);

            *tstate.treeptr = new CEvalNodeNum {
                tstate.treeptr,
                tstate.parent,
                res_offset,
                false,
            };

            tstate.output = res_offset;

            ++token_it;
            break;
        }
        case sxm::Token_Kind::PriorityOpen: {
            auto lnode = new CEvalNodePriority {
                tstate.treeptr,
                tstate.parent,
                nullptr
            };

            *tstate.treeptr = lnode;

            tstate.parent = lnode;
            tstate.treeptr = &lnode->child;

            ++token_it;
            break;
        }
        case sxm::Token_Kind::PriorityClose: {
            sxm::CEvalNode *target = (*tstate.treeptr)->parent;

            while (target->kind != sxm::ASTNode_Kind::Priority) {
                target = target->parent;
            }

            tstate.treeptr = target->slot;
            tstate.parent = target->parent;

            auto lnode = static_cast<sxm::CEvalNodePriority *>(target);

            std::optional<sxm::CEvalError> fold_report = fold_expr(lnode->child, target->parent, tstate);

            if (fold_report.has_value()) {
                tstate.destroy();

                return std::unexpected { fold_report.value(), };
            }

            ++token_it;
            break;
        }
        case sxm::Token_Kind::UnaryOp: {
            auto lnode = new CEvalNodeUOp {
                tstate.treeptr,
                tstate.parent,
                uop_let(token->section_start),
                nullptr,
            };

            *tstate.treeptr = lnode;

            tstate.parent = lnode;
            tstate.treeptr = &lnode->child;

            ++token_it;
            break;
        }
        case sxm::Token_Kind::BinaryOp: {
            sxm::BinaryOp operation = sxm::biop_let(token->section_start, token->section_end);
            unsigned short operation_priority = sxm::biop_priority_let(operation);

            auto target = *tstate.treeptr;
            while (target->parent != nullptr) {
                if (target->parent->kind == sxm::ASTNode_Kind::UOp) {
                    target = target->parent;

                    continue;
                } else if (target->parent->kind == sxm::ASTNode_Kind::BiOp) {
                    const auto target_parent = static_cast<sxm::CEvalNodeBiOp *>(target->parent);
                    const unsigned short target_parent_priority = sxm::biop_priority_let(target_parent->operation);

                    if (target_parent_priority >= operation_priority) {
                        target = target_parent;

                        continue;
                    } else {
                        break;
                    }
                }

                break;
            }

            auto lnode = new CEvalNodeBiOp {
                target->slot,
                target->parent,
                operation,
                target,
                nullptr,
            };

            *lnode->slot = lnode;
            target->parent = lnode;
            target->slot = &lnode->left;

            tstate.parent = lnode;
            tstate.treeptr = &lnode->right;

            if (!(operation == sxm::BinaryOp::Assign && target->kind == sxm::ASTNode_Kind::Ref)) {
                std::optional<sxm::CEvalError> fold_report = fold_expr(target, lnode, tstate);

                if (fold_report.has_value()) {
                    tstate.destroy();

                    return std::unexpected { fold_report.value(), };
                }
            }

            ++token_it;
            break;
        }
        case sxm::Token_Kind::Reference: {
            Range<const char> name { token->section_start, token->section_end, };

            ++token_it;

            if (!token.has_value()) {
                tstate.destroy();

                return std::unexpected(token.error());
            }

            if (token->kind == sxm::Token_Kind::FnOpen) {
                ++token_it;

                if (!token.has_value()) {
                    tstate.destroy();

                    return std::unexpected(token.error());
                }

                if (token->kind == sxm::Token_Kind::FnClose) {
                    std::optional<sxm::numfn::Def> fn_def = numfn::hmap.find(name);

                    if (!fn_def.has_value()) {
                        tstate.destroy();

                        return std::unexpected {
                            sxm::EvalError {
                                sxm::EvalError_Kind::FnUndef,
                                "Undefined Function " + std::string { name.p_start, name.p_end, }
                            }
                        };
                    }

                    if (!fn_def->validator(0)) {
                        tstate.destroy();

                        return std::unexpected {
                            sxm::EvalError {
                                sxm::EvalError_Kind::FnEval,
                                "Failed to evaluate function " + std::string { name.p_start, name.p_end, }
                            }
                        };
                    }

                    sxm::CEvalOffset res_offset = tstate.const_track.push(fn_def->fn_raw({}));

                    *tstate.treeptr = new CEvalNodeNum {
                        tstate.treeptr,
                        tstate.parent,
                        res_offset,
                        false,
                    };

                    tstate.output = res_offset;

                    ++token_it;
                } else {
                    auto lnode = new CEvalNodeFn {
                        tstate.treeptr,
                        tstate.parent,
                        { name.p_start, name.p_end, },
                        std::vector<CEvalNode *>(1),
                    };

                    *tstate.treeptr = lnode;

                    tstate.treeptr = lnode->arg_list.data();
                    tstate.parent = lnode;

                    tstate.arg_track.push();
                }

                break;
            }

            {
                *tstate.treeptr = new sxm::CEvalNodeRef {
                    tstate.treeptr,
                    tstate.parent,
                    name,
                };

                break;
            }
        }
        case sxm::Token_Kind::FnComma: {
            sxm::CEvalNode *target = tstate.parent;

            while (target->kind != sxm::ASTNode_Kind::Fn) {
                target = target->parent;
            }

            auto lnode = static_cast<sxm::CEvalNodeFn *>(target);

            std::optional<sxm::CEvalError> fold_report = fold_expr(lnode->arg_list.back(), lnode, tstate);

            if (fold_report.has_value()) {
                tstate.destroy();

                return std::unexpected { fold_report.value() };
            }

            {
                auto lchild = static_cast<sxm::CEvalNodeNum *>(lnode->arg_list.back());

                tstate.inst_list.push({
                    .kind = sxm::CEvalInstr_Kind::ArgPush,
                    .left { .offset { lchild->offset } },
                    .right { },
                    .output { },
                });

                tstate.arg_track.push();
                tstate.state_track.free_num(lchild);
            }

            {
                sxm::CEvalNode **arg_data_before = lnode->arg_list.data();

                lnode->arg_list.emplace_back(nullptr);

                sxm::CEvalNode **arg_data_after = lnode->arg_list.data();

                if (arg_data_before != arg_data_after) {
                    sxm::CEvalNode **arg_end = arg_data_after + lnode->arg_list.size() - 1;

                    for (sxm::CEvalNode **arg_now = arg_data_after; arg_now < arg_end; ++arg_now) {
                        (*arg_now)->slot = arg_now;
                    }
                }
            }

            tstate.parent = lnode;
            tstate.treeptr = lnode->arg_list.data() + lnode->arg_list.size() - 1;

            ++token_it;
            break;
        }
        case sxm::Token_Kind::FnClose: {
            sxm::CEvalNode *target = tstate.parent;

            while (target->kind != sxm::ASTNode_Kind::Fn) {
                target = target->parent;
            }

            auto lnode = static_cast<sxm::CEvalNodeFn *>(target);

            std::optional<sxm::CEvalError> fold_report = fold_expr(lnode->arg_list.back(), lnode, tstate);

            if (fold_report.has_value()) {
                tstate.destroy();

                return std::unexpected { fold_report.value() };
            }

            {
                auto lchild = static_cast<sxm::CEvalNodeNum *>(lnode->arg_list.back());

                tstate.inst_list.push({
                    .kind = sxm::CEvalInstr_Kind::ArgPush,
                    .left { .offset { lchild->offset } },
                    .right { },
                    .output { },
                });

                tstate.arg_track.push();
                tstate.state_track.free_num(lchild);
            }

            std::optional<sxm::numfn::Def> fn_def = sxm::numfn::hmap.find(lnode->name);

            if (!fn_def.has_value()) {
                Range<const char> name = lnode->name;

                tstate.destroy();

                return std::unexpected {
                    sxm::EvalError {
                        sxm::EvalError_Kind::FnUndef,
                        "Undefined Function " + std::string { name.p_start, name.p_end, }
                    }
                };
            }

            if (!fn_def->validator(lnode->arg_list.size())) {
                Range<const char> name = lnode->name;

                tstate.destroy();

                return std::unexpected {
                    sxm::EvalError {
                        sxm::EvalError_Kind::FnEval,
                        "Failed to evaluate function " + std::string { name.p_start, name.p_end, }
                    }
                };
            }

            sxm::CEvalNode **slot = lnode->slot;
            sxm::CEvalNode *parent = lnode->parent;

            if (fn_status_cexpr(lnode)) {
                tstate.inst_list.shrink(lnode->arg_list.size());
                tstate.arg_track.take_silent(lnode->arg_list.size());

                sxm::NumValue *argv_start = new sxm::NumValue[lnode->arg_list.size()];
                sxm::NumValue *argv_end = argv_start + lnode->arg_list.size();

                for (std::size_t index = 0; index < lnode->arg_list.size(); ++index) {
                    auto arg_num = reinterpret_cast<sxm::CEvalNodeNum *>(lnode->arg_list[index]);

                    tstate.const_track.free_num(arg_num);
                    *(argv_start + index) = tstate.const_track.at(arg_num->offset);
                }

                sxm::NumValue res = fn_def->fn_raw({ argv_start, argv_end });
                sxm::CEvalOffset res_offset = tstate.const_track.push(res);

                *slot = new sxm::CEvalNodeNum {
                    slot,
                    parent,
                    res_offset,
                    false
                };

                tstate.output = res_offset;

                delete[] argv_start;
            } else {
                sxm::CEvalOffset res_offset = tstate.state_track.alloc();

                tstate.inst_list.push({
                    .kind = sxm::CEvalInstr_Kind::Call,
                    .left { .fn = fn_def.value().fn_raw },
                    .right { .length = lnode->arg_list.size() },
                    .output { .offset { res_offset } },
                });

                *slot = new sxm::CEvalNodeNum {
                    slot,
                    parent,
                    res_offset,
                    false
                };

                tstate.output = res_offset;
                tstate.arg_track.take(lnode->arg_list.size());
            }

            tstate.treeptr = slot;
            tstate.parent = parent;

            delete lnode;

            ++token_it;
            break;
        }
        }
    }
}

sxm::NumValue sxm::CEvalDeclaration::eval(Range<sxm::NumValue> sxmvar_local) const noexcept {
    void *memory = std::malloc((this->state_size + this->arg_size) * sizeof(sxm::NumValue));

    sxm::NumValue *state_list = reinterpret_cast<sxm::NumValue *>(memory);
    sxm::NumValue *arg_list_now = state_list + this->state_size;

    if (sxmvar_local.p_start != nullptr) {
        std::memcpy(state_list, sxmvar_local.p_start, sxmvar_local.size() * sizeof(sxm::NumValue));
    }

    for (sxm::CEvalInstr *inst_now = this->inst_list.p_start;; ++inst_now) {
        sxm::NumValue *output = inst_now->output.offset.resolve(this->const_list.p_start, state_list);

        switch (inst_now->kind) {
        case sxm::CEvalInstr_Kind::Ret: {
            sxm::NumValue left = *inst_now->left.offset.resolve(this->const_list.p_start, state_list);

            std::free(memory);

            return left;
        }

        case sxm::CEvalInstr_Kind::Call: {
            *output = inst_now->left.fn({ arg_list_now - inst_now->right.length, arg_list_now });

            arg_list_now -= inst_now->right.length;

            break;
        }
        case sxm::CEvalInstr_Kind::ArgPush: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);

            *arg_list_now = *left;

            ++arg_list_now;

            break;
        }

        case sxm::CEvalInstr_Kind::Set: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);

            *output = *left;

            break;
        }
        case sxm::CEvalInstr_Kind::Neg: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::neg(left);

            break;
        }

        case sxm::CEvalInstr_Kind::Add: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::add(left, right);

            break;
        }
        case sxm::CEvalInstr_Kind::Sub: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::sub(left, right);

            break;
        }
        case sxm::CEvalInstr_Kind::Mul: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::mul(left, right);

            break;
        }
        case sxm::CEvalInstr_Kind::Div: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::div(left, right);

            break;
        }
        case sxm::CEvalInstr_Kind::Mod: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::mod(left, right);

            break;
        }
        case sxm::CEvalInstr_Kind::Pow: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::pow(left, right);

            break;
        }
        case sxm::CEvalInstr_Kind::Eq: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::eq(left, right);

            break;
        }
        case sxm::CEvalInstr_Kind::UEq: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::ueq(left, right);

            break;
        }
        case sxm::CEvalInstr_Kind::GTE: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::gte(left, right);

            break;
        }
        case sxm::CEvalInstr_Kind::LTE: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::lte(left, right);

            break;
        }
        case sxm::CEvalInstr_Kind::LT: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::lt(left, right);

            break;
        }
        case sxm::CEvalInstr_Kind::GT: {
            sxm::NumValue *left = inst_now->left.offset.resolve(this->const_list.p_start, state_list);
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = sxm::NumValue::gt(left, right);

            break;
        }
        case sxm::CEvalInstr_Kind::Assign: {
            sxm::NumValue *right = inst_now->right.offset.resolve(this->const_list.p_start, state_list);

            *output = *right;

            break;
        }
        default: {
            std::unreachable();
        }
        }
    }
}

sxm::CEvalUnit sxm::CEvalDeclaration::init() const noexcept {
    std::size_t bytesize_inst = this->inst_list.size() * sizeof(sxm::CEvalInstr);
    std::size_t bytesize_state = (this->arg_size + this->state_size) * sizeof(sxm::NumValue);

    void *memory = std::malloc(bytesize_inst + bytesize_state);

    Range<sxm::CEvalInstr> inst_list {
        reinterpret_cast<sxm::CEvalInstr *>(memory),
        reinterpret_cast<sxm::CEvalInstr *>(memory) + this->inst_list.size(),
    };

    Range<sxm::NumValue> state_list {
        reinterpret_cast<sxm::NumValue *>(inst_list.p_end),
        reinterpret_cast<sxm::NumValue *>(inst_list.p_end) + this->state_size,
    };

    Range<sxm::NumValue> arg_list {
        state_list.p_end,
        state_list.p_end + this->arg_size,
    };

    for (std::size_t index = 0;; ++index) {
        sxm::CEvalInstr *src_inst = this->inst_list.p_start + index;
        sxm::CEvalInstr *dest_inst = inst_list.p_start + index;

        dest_inst->kind = src_inst->kind;

        switch (src_inst->kind) {
        case sxm::CEvalInstr_Kind::Ret: {
            dest_inst->left.value = src_inst->left.offset.resolve(this->const_list.p_start, state_list.p_start);

            return {
                arg_list,
                state_list,
                inst_list,
            };
        }

        case sxm::CEvalInstr_Kind::Call: {
            dest_inst->left.fn = src_inst->left.fn;
            dest_inst->right.offset = src_inst->right.offset;
            dest_inst->output.value = src_inst->output.offset.resolve(this->const_list.p_start, state_list.p_start);

            break;
        }
        case sxm::CEvalInstr_Kind::ArgPush: {
            dest_inst->left.value = src_inst->left.offset.resolve(this->const_list.p_start, state_list.p_start);

            break;
        }

        case sxm::CEvalInstr_Kind::Set:
        case sxm::CEvalInstr_Kind::Neg: {
            dest_inst->left.value = src_inst->left.offset.resolve(this->const_list.p_start, state_list.p_start);
            dest_inst->output.value = src_inst->output.offset.resolve(this->const_list.p_start, state_list.p_start);

            break;
        }

        case sxm::CEvalInstr_Kind::Add:
        case sxm::CEvalInstr_Kind::Sub:
        case sxm::CEvalInstr_Kind::Mul:
        case sxm::CEvalInstr_Kind::Div:
        case sxm::CEvalInstr_Kind::Mod:
        case sxm::CEvalInstr_Kind::Pow:
        case sxm::CEvalInstr_Kind::Eq:
        case sxm::CEvalInstr_Kind::UEq:
        case sxm::CEvalInstr_Kind::GTE:
        case sxm::CEvalInstr_Kind::LTE:
        case sxm::CEvalInstr_Kind::LT:
        case sxm::CEvalInstr_Kind::GT: {
            dest_inst->left.value = src_inst->left.offset.resolve(this->const_list.p_start, state_list.p_start);
            dest_inst->right.value = src_inst->right.offset.resolve(this->const_list.p_start, state_list.p_start);
            dest_inst->output.value = src_inst->output.offset.resolve(this->const_list.p_start, state_list.p_start);

            break;
        }

        case sxm::CEvalInstr_Kind::Assign: {
            dest_inst->right.value = src_inst->right.offset.resolve(this->const_list.p_start, state_list.p_start);
            dest_inst->output.value = src_inst->output.offset.resolve(this->const_list.p_start, state_list.p_start);

            break;
        }
        default: {
            std::unreachable();
        }
        }
    }
}

sxm::NumValue sxm::CEvalUnit::eval() noexcept {
    sxm::NumValue *arg_list_now = this->arg_list.p_start;

    for (sxm::CEvalInstr *inst_now = this->inst_list.p_start;; ++inst_now) {
        switch (inst_now->kind) {
        case sxm::CEvalInstr_Kind::Ret: {
            return *inst_now->left.value;
        }

        case sxm::CEvalInstr_Kind::Call: {
            *inst_now->output.value = inst_now->left.fn({
                arg_list_now - inst_now->right.length,
                arg_list_now
            });

            arg_list_now -= inst_now->right.length;

            break;
        }
        case sxm::CEvalInstr_Kind::ArgPush: {
            *arg_list_now = *inst_now->left.value;

            ++arg_list_now;

            break;
        }

        case sxm::CEvalInstr_Kind::Set: {
            *inst_now->output.value = *inst_now->left.value;

            break;
        }
        case sxm::CEvalInstr_Kind::Neg: {
            *inst_now->output.value = sxm::NumValue::neg(inst_now->left.value);

            break;
        }

        case sxm::CEvalInstr_Kind::Add: {
            *inst_now->output.value = sxm::NumValue::add(inst_now->left.value, inst_now->right.value);

            break;
        }
        case sxm::CEvalInstr_Kind::Sub: {
            *inst_now->output.value = sxm::NumValue::sub(inst_now->left.value, inst_now->right.value);

            break;
        }
        case sxm::CEvalInstr_Kind::Mul: {
            *inst_now->output.value = sxm::NumValue::mul(inst_now->left.value, inst_now->right.value);

            break;
        }
        case sxm::CEvalInstr_Kind::Div: {
            *inst_now->output.value = sxm::NumValue::div(inst_now->left.value, inst_now->right.value);

            break;
        }
        case sxm::CEvalInstr_Kind::Mod: {
            *inst_now->output.value = sxm::NumValue::mod(inst_now->left.value, inst_now->right.value);

            break;
        }
        case sxm::CEvalInstr_Kind::Pow: {
            *inst_now->output.value = sxm::NumValue::pow(inst_now->left.value, inst_now->right.value);

            break;
        }
        case sxm::CEvalInstr_Kind::Eq: {
            *inst_now->output.value = sxm::NumValue::eq(inst_now->left.value, inst_now->right.value);

            break;
        }
        case sxm::CEvalInstr_Kind::UEq: {
            *inst_now->output.value = sxm::NumValue::ueq(inst_now->left.value, inst_now->right.value);

            break;
        }
        case sxm::CEvalInstr_Kind::GTE: {
            *inst_now->output.value = sxm::NumValue::gte(inst_now->left.value, inst_now->right.value);

            break;
        }
        case sxm::CEvalInstr_Kind::LTE: {
            *inst_now->output.value = sxm::NumValue::lte(inst_now->left.value, inst_now->right.value);

            break;
        }
        case sxm::CEvalInstr_Kind::LT: {
            *inst_now->output.value = sxm::NumValue::lt(inst_now->left.value, inst_now->right.value);

            break;
        }
        case sxm::CEvalInstr_Kind::GT: {
            *inst_now->output.value = sxm::NumValue::gt(inst_now->left.value, inst_now->right.value);

            break;
        }
        case sxm::CEvalInstr_Kind::Assign: {
            *inst_now->output.value = *inst_now->right.value;

            break;
        }
        default: {
            std::unreachable();
        }
        }
    }
}
