#include "sxm/eval/linked.hpp"
#include "algo/search_map/h.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/ast/fnmap.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/parser/parse.hpp"
#include "sxm/parser/token.hpp"
#include <expected>

sxm::LEvalNodeUOp::~LEvalNodeUOp() {
    delete this->child;
};

sxm::LEvalNodeBiOp::~LEvalNodeBiOp() {
    delete this->left;
    delete this->right;
};

sxm::LEvalNodePriority::~LEvalNodePriority() {
    delete this->child;
};

sxm::LEvalNodeFn::~LEvalNodeFn() {
    for (auto arg : this->arg_list) {
        delete arg;
    }
};

namespace {

struct TraverseState {
    sxm::LEvalNode *parent;
    sxm::LEvalNode **treeptr;
};

inline sxm::NumValue numvalue_let_uop(sxm::LEvalNodeUOp *node) {
    const auto lchild = static_cast<sxm::LEvalNodeNum *>(node->child);

    switch (node->operation) {
    case sxm::UnaryOp::Neg: {
        return sxm::NumValue::neg(&lchild->value);
    }
    case sxm::UnaryOp::Pos: {
        return lchild->value;
    }
    }

    throw std::runtime_error("unknown biop");
}

inline sxm::NumValue numvalue_let_biop(sxm::LEvalNodeBiOp *node) {
    const auto lleft = static_cast<sxm::LEvalNodeNum *>(node->left);
    const auto lright = static_cast<sxm::LEvalNodeNum *>(node->right);

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

    throw std::runtime_error("unknown biop");
}

std::optional<sxm::LEvalError> fold_expr(sxm::LEvalNode *target, sxm::LEvalNode *limit, HMap<sxm::NumValue> *val_hmap) {
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
            auto lnode = static_cast<sxm::LEvalNodeRef *>(target);

            std::optional<sxm::NumValue> ref_value = val_hmap->find(lnode->name);

            if (!ref_value.has_value()) {
                return sxm::EvalError {
                    sxm::EvalError_Kind::VarUndef,
                    "Undefined Variable " + std::string{lnode->name.p_start, lnode->name.p_end}
                } ;
            }

            sxm::LEvalNode **slot = target->slot;
            sxm::LEvalNode *parent = target->parent;

            delete lnode;

            *slot = new sxm::LEvalNodeNum {
                slot,
                parent,
                ref_value.value(),
            };

            target = parent;

            break;
        }
        case sxm::ASTNode_Kind::UOp: {
            auto lnode = static_cast<sxm::LEvalNodeUOp *>(target);

            if (lnode->child->kind != sxm::ASTNode_Kind::Num) {
                target = lnode->child;

                break;
            }

            sxm::LEvalNode **slot = target->slot;
            sxm::LEvalNode *parent = target->parent;
            auto value = numvalue_let_uop(lnode);

            delete lnode;

            *slot = new sxm::LEvalNodeNum {
                slot,
                parent,
                value,
            };


            target = parent;

            break;
        }
        case sxm::ASTNode_Kind::BiOp: {
            auto lnode = static_cast<sxm::LEvalNodeBiOp *>(target);

            if (lnode->operation == sxm::BinaryOp::Assign && lnode->left->kind == sxm::ASTNode_Kind::Ref) {
                if (lnode->right->kind != sxm::ASTNode_Kind::Num) {
                    target = lnode->right;

                    break;
                }

                auto lleft = static_cast<sxm::LEvalNodeRef *>(lnode->left);
                auto lright = static_cast<sxm::LEvalNodeNum *>(lnode->right);

                val_hmap->insert(lleft->name, lright->value);

                sxm::LEvalNode **slot = target->slot;
                sxm::LEvalNode *parent = target->parent;
                sxm::NumValue value = lright->value;

                delete target;

                *slot = new sxm::LEvalNodeNum {
                    slot,
                    parent,
                    value,
                };

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

                sxm::LEvalNode **slot = target->slot;
                sxm::LEvalNode *parent = target->parent;
                sxm::NumValue value = numvalue_let_biop(lnode);

                delete lnode;

                *slot = new sxm::LEvalNodeNum {
                    slot,
                    parent,
                    value,
                };

                target = parent;
            }

            break;
        }
        case sxm::ASTNode_Kind::Priority: {
            auto lnode = static_cast<sxm::LEvalNodePriority *>(target);

            if (lnode->child->kind != sxm::ASTNode_Kind::Num) {
                target = lnode->child;

                break;
            }

            auto lchild = static_cast<sxm::LEvalNodeNum *>(lnode->child);

            sxm::LEvalNode **slot = lnode->slot;
            sxm::LEvalNode *parent = lnode->parent;
            sxm::NumValue value = lchild->value;

            delete lnode;

            *slot = new sxm::LEvalNodeNum {
                slot,
                parent,
                value,
            };

            target = parent;

            break;
        }
        }
    } while (target != limit);

    return std::nullopt;
}

}

std::expected<sxm::NumValue, sxm::LEvalError> sxm::eval_linked(const char *src, HMap<sxm::NumValue> *val_hmap) {
    sxm::LEvalNode *root = nullptr;

    TraverseState tstate = {
        .parent = nullptr,
        .treeptr = &root,
    };

    auto token_gen = sxm::parse(src);
    auto token_gen_now = token_gen.begin();

    while (true) {
        std::expected<sxm::Token, sxm::ParserError> &token_now = *token_gen_now;

        if (!token_now.has_value()) {
            delete root;

            return std::unexpected { token_now.error(), };
        }

        switch (token_now->kind) {
        case sxm::Token_Kind::Space:
        case sxm::Token_Kind::FnOpen:
            std::unreachable();
        case sxm::Token_Kind::End: {
            std::optional<sxm::LEvalError> fold_report = fold_expr(root, nullptr, val_hmap);

            if (fold_report.has_value()) {
                delete root;

                return std::unexpected { fold_report.value(), };
            }

            sxm::NumValue result = static_cast<sxm::LEvalNodeNum *>(root)->value;

            delete root;

            return result;
        }
        case sxm::Token_Kind::NumberInt: {
            *tstate.treeptr = new LEvalNodeNum {
                tstate.treeptr,
                tstate.parent,
                sxm::NumValue::parse_int(token_now->section_start, token_now->section_end),
            };

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::NumberReal: {
            *tstate.treeptr = new LEvalNodeNum {
                tstate.treeptr,
                tstate.parent,
                sxm::NumValue::parse_real(token_now->section_start, token_now->section_end),
            };

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::PriorityOpen: {
            auto lnode = new LEvalNodePriority {
                tstate.treeptr,
                tstate.parent,
                nullptr
            };

            *tstate.treeptr = lnode;

            tstate.parent = lnode;
            tstate.treeptr = &lnode->child;

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::PriorityClose: {
            sxm::LEvalNode *target = (*tstate.treeptr)->parent;

            while (target->kind != sxm::ASTNode_Kind::Priority) {
                target = target->parent;
            }

            tstate.treeptr = target->slot;
            tstate.parent = target->parent;

            auto lnode = static_cast<sxm::LEvalNodePriority *>(target);

            std::optional<sxm::LEvalError> fold_report = fold_expr(lnode->child, target->parent, val_hmap);

            if (fold_report.has_value()) {
                delete root;

                return std::unexpected { fold_report.value(), };
            }

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::UnaryOp: {
            auto lnode = new LEvalNodeUOp {
                tstate.treeptr,
                tstate.parent,
                uop_let(token_now->section_start),
                nullptr,
            };

            *tstate.treeptr = lnode;

            tstate.parent = lnode;
            tstate.treeptr = &lnode->child;

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::BinaryOp: {
            sxm::BinaryOp operation = sxm::biop_let(token_now->section_start, token_now->section_end);
            unsigned short operation_priority = sxm::biop_priority_let(operation);

            auto target = *tstate.treeptr;
            while (target->parent != nullptr) {
                if (target->parent->kind == sxm::ASTNode_Kind::UOp) {
                    target = target->parent;

                    continue;
                } else if (target->parent->kind == sxm::ASTNode_Kind::BiOp) {
                    const auto target_parent = static_cast<sxm::LEvalNodeBiOp *>(target->parent);
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

            auto lnode = new LEvalNodeBiOp {
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
                std::optional<sxm::LEvalError> fold_report = fold_expr(target, lnode, val_hmap);

                if (fold_report.has_value()) {
                    delete root;

                    return std::unexpected { fold_report.value(), };
                }
            }

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::Reference: {
            Range<const char> name { token_now->section_start, token_now->section_end, };

            ++token_gen_now;

            if (!token_now.has_value()) {
                delete root;

                return std::unexpected(token_now.error());
            }

            if (token_now->kind == sxm::Token_Kind::FnOpen) {
                ++token_gen_now;

                if (!token_now.has_value()) {
                    delete root;

                    return std::unexpected(token_now.error());
                }

                if (token_now->kind == sxm::Token_Kind::FnClose) {
                    std::optional<sxm::numfn::Def> fn_def = sxm::numfn::hmap.find(name);

                    if (!fn_def.has_value()) {
                        delete root;

                        return std::unexpected {
                            sxm::EvalError {
                                sxm::EvalError_Kind::FnUndef,
                                "Undefined Function " + std::string { name.p_start, name.p_end, }
                            }
                        };
                    }

                    if (!fn_def->validator(0)) {
                        delete root;

                        return std::unexpected {
                            sxm::EvalError {
                                sxm::EvalError_Kind::FnEval,
                                "Failed to evaluate function " + std::string { name.p_start, name.p_end, }
                            }
                        };
                    }

                    *tstate.treeptr = new LEvalNodeNum {
                        tstate.treeptr,
                        tstate.parent,
                        fn_def->fn_leval({ nullptr, nullptr }),
                    };

                    ++token_gen_now;
                } else {
                    auto lnode = new LEvalNodeFn {
                        tstate.treeptr,
                        tstate.parent,
                        { name.p_start, name.p_end, },
                        std::vector<LEvalNode *>(1),
                    };

                    *tstate.treeptr = lnode;

                    tstate.treeptr = lnode->arg_list.data();
                    tstate.parent = lnode;
                }

                break;
            }

            {
                *tstate.treeptr = new sxm::LEvalNodeRef {
                    tstate.treeptr,
                    tstate.parent,
                    name,
                };

                break;
            }
        }
        case sxm::Token_Kind::FnComma: {
            sxm::LEvalNode *target = tstate.parent;

            while (target->kind != sxm::ASTNode_Kind::Fn) {
                target = target->parent;
            }

            auto lnode = static_cast<sxm::LEvalNodeFn *>(target);

            std::optional<sxm::LEvalError> fold_report = fold_expr(lnode->arg_list.back(), lnode, val_hmap);

            if (fold_report.has_value()) {
                delete root;

                return std::unexpected { fold_report.value() };
            }

            {
                sxm::LEvalNode **arg_data_before = lnode->arg_list.data();

                lnode->arg_list.emplace_back(nullptr);

                sxm::LEvalNode **arg_data_after = lnode->arg_list.data();

                if (arg_data_before != arg_data_after) {
                    sxm::LEvalNode **arg_end = arg_data_after + lnode->arg_list.size() - 1;

                    for (sxm::LEvalNode **arg_now = arg_data_after; arg_now < arg_end; ++arg_now) {
                        (*arg_now)->slot = arg_now;
                    }
                }
            }

            tstate.parent = lnode;
            tstate.treeptr = lnode->arg_list.data() + lnode->arg_list.size() - 1;

            ++token_gen_now;
            break;
        }
        case sxm::Token_Kind::FnClose: {
            sxm::LEvalNode *target = tstate.parent;

            while (target->kind != sxm::ASTNode_Kind::Fn) {
                target = target->parent;
            }

            auto lnode = static_cast<sxm::LEvalNodeFn *>(target);

            std::optional<sxm::LEvalError> fold_report = fold_expr(lnode->arg_list.back(), lnode, val_hmap);

            if (fold_report.has_value()) {
                delete root;

                return std::unexpected { fold_report.value() };
            }

            std::optional<sxm::numfn::Def> fn_def = sxm::numfn::hmap.find(lnode->name);

            if (!fn_def.has_value()) {
                Range<const char> name = lnode->name;

                delete root;

                return std::unexpected {
                    sxm::EvalError {
                        sxm::EvalError_Kind::FnUndef,
                        "Undefined Function " + std::string { name.p_start, name.p_end, }
                    }
                };
            }

            if (!fn_def->validator(lnode->arg_list.size())) {
                Range<const char> name = lnode->name;

                delete root;

                return std::unexpected {
                    sxm::EvalError {
                        sxm::EvalError_Kind::FnEval,
                        "Failed to evaluate function " + std::string { name.p_start, name.p_end, }
                    }
                };
            }

            sxm::LEvalNode **slot = lnode->slot;
            sxm::LEvalNode *parent = lnode->parent;

            sxm::NumValue fn_value = fn_def->fn_leval({
                reinterpret_cast<sxm::LEvalNodeNum **>(lnode->arg_list.data()),
                reinterpret_cast<sxm::LEvalNodeNum **>(lnode->arg_list.data() + lnode->arg_list.size()),
            });

            delete lnode;

            *slot = new sxm::LEvalNodeNum {
                slot,
                parent,
                fn_value,
            };

            tstate.treeptr = slot;
            tstate.parent = parent;

            ++token_gen_now;
            break;
        }
        }
    }
}
