#include "sxm/eval/pure.hpp"
#include "sxm/debug/tostr.hpp"
#include "sxm/ast/fnmap.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/parser/parse.hpp"
#include <cstddef>
#include <expected>
#include <string>
#include <vector>

sxm::PEvalNodePriority::~PEvalNodePriority() noexcept {
    delete this->child;
}

sxm::PEvalNodeBiOp::~PEvalNodeBiOp() noexcept {
    delete this->left;
    delete this->right;
}

sxm::PEvalNodeUOp::~PEvalNodeUOp() noexcept {
    delete this->child;
}

sxm::PEvalNodeRef::~PEvalNodeRef() noexcept {
    delete[] this->name.p_start;
}

sxm::PEvalNodeFn::~PEvalNodeFn() noexcept {
    delete[] this->name.p_start;

    for (sxm::PEvalNode *arg : this->arg_list) {
        delete arg;
    }
}

namespace {

struct StackNode {
    bool check;
    sxm::PEvalNode *ast;
    sxm::NumValue *output;
    sxm::NumValue *value_list;

    StackNode(bool check, sxm::PEvalNode *ast, sxm::NumValue *output, sxm::NumValue *value_list) noexcept
        : check(check), ast(ast), output(output), value_list(value_list) { }

    StackNode(StackNode &src) noexcept
        : check(src.check), ast(src.ast), output(src.output), value_list(src.value_list) { }

    StackNode(StackNode &&src) noexcept
        : check(src.check), ast(src.ast), output(src.output), value_list(src.value_list) {

        src.value_list = nullptr;
    }

    StackNode& operator=(StackNode &src) noexcept {
        this->check = src.check;
        this->ast = src.ast;
        this->output = src.output;
        this->value_list = src.value_list;

        return *this;
    }

    StackNode& operator=(StackNode &&src) noexcept {
        this->check = src.check;
        this->ast = src.ast;
        this->output = src.output;
        this->value_list = src.value_list;

        src.value_list = nullptr;

        return *this;
    }

    ~StackNode() {
        delete[] this->value_list;
    }
};

inline void stack_handle(std::vector<StackNode> *stack, sxm::PEvalNode *node, sxm::NumValue *output) {
    while (node->kind == sxm::ASTNode_Kind::Priority) {
        node = static_cast<sxm::PEvalNodePriority *>(node)->child;
    }

    switch (node->kind) {
    case sxm::ASTNode_Kind::Num: {
        *output = static_cast<sxm::PEvalNodeNum *>(node)->value;

        break;
    }
    case sxm::ASTNode_Kind::Ref: {
        stack->emplace_back(
            true,
            node,
            output,
            nullptr
        );

        break;
    }
    default: {
        stack->emplace_back(
            false,
            node,
            output,
            nullptr
        );

        break;
    }
    }
}

inline void handle_uop(StackNode *snode, sxm::PEvalNodeUOp *nodeop) {
    switch (nodeop->operation) {
    case sxm::UnaryOp::Neg: {
        *snode->output = sxm::NumValue::neg(snode->value_list);

        break;
    }
    case sxm::UnaryOp::Pos: {
        *snode->output = *snode->value_list;

        break;
    }
    }
}

inline void handle_biop(StackNode *snode, sxm::PEvalNodeBiOp *nodeop, HMap<sxm::NumValue> *val_hmap) {
    switch (nodeop->operation) {
    case sxm::BinaryOp::Mul: {
        *snode->output = sxm::NumValue::mul(snode->value_list, snode->value_list + 1);

        break;
    }
    case sxm::BinaryOp::Div: {
        *snode->output = sxm::NumValue::div(snode->value_list, snode->value_list + 1);

        break;
    }
    case sxm::BinaryOp::Sub: {
        *snode->output = sxm::NumValue::sub(snode->value_list, snode->value_list + 1);

        break;
    }
    case sxm::BinaryOp::Add: {
        *snode->output = sxm::NumValue::add(snode->value_list, snode->value_list + 1);

        break;
    }
    case sxm::BinaryOp::Mod: {
        *snode->output = sxm::NumValue::mod(snode->value_list, snode->value_list + 1);

        break;
    }
    case sxm::BinaryOp::Pow: {
        *snode->output = sxm::NumValue::pow(snode->value_list, snode->value_list + 1);

        break;
    }
    case sxm::BinaryOp::Eq: {
        *snode->output = sxm::NumValue::eq(snode->value_list, snode->value_list + 1);

        break;
    }
    case sxm::BinaryOp::UEq: {
        *snode->output = sxm::NumValue::ueq(snode->value_list, snode->value_list + 1);

        break;
    }
    case sxm::BinaryOp::LT: {
        *snode->output = sxm::NumValue::lt(snode->value_list, snode->value_list + 1);

        break;
    }
    case sxm::BinaryOp::LTE: {
        *snode->output = sxm::NumValue::lte(snode->value_list, snode->value_list + 1);

        break;
    }
    case sxm::BinaryOp::GT: {
        *snode->output = sxm::NumValue::gt(snode->value_list, snode->value_list + 1);

        break;
    }
    case sxm::BinaryOp::GTE: {
        *snode->output = sxm::NumValue::gte(snode->value_list, snode->value_list + 1);

        break;
    }
    case sxm::BinaryOp::Assign: {
        if (nodeop->left->kind == sxm::ASTNode_Kind::Ref) {
            auto nodeop_left = static_cast<sxm::PEvalNodeRef *>(nodeop->left);

            val_hmap->insert(nodeop_left->name, snode->value_list[1]);
        }

        *snode->output = snode->value_list[1];
    }
    }
}

inline Range<const char> name_new(const char *src_now, const char *src_end) {
    std::size_t size = src_end - src_now;
    char *const name_start = new char[size];
    std::memcpy(name_start, src_now, size);

    return { name_start, name_start + size, };
}

}

std::expected<sxm::PEvalNode *, sxm::ParserError> sxm::eval_pure_build(const char *src) {
    sxm::PEvalNode *root = nullptr;
    sxm::PEvalNode *parent = nullptr;
    sxm::PEvalNode **pointer = &root;

    auto token_gen = sxm::parse(src);
    auto token_gen_now = token_gen.begin();

    while (true) {
        std::expected<sxm::Token, sxm::ParserError> &token_now = *token_gen_now;

        if (!token_now.has_value()) {
            delete root;

            return std::unexpected { token_now.error() };
        }

        switch (token_now->kind) {
        case sxm::Token_Kind::Space:
        case sxm::Token_Kind::FnOpen: {
            std::unreachable();
        }
        case sxm::Token_Kind::End: {
            root->slot = nullptr;

            return root;
        }
        case sxm::Token_Kind::Reference: {
            Range<const char> name = name_new(token_now->section_start, token_now->section_end);

            ++token_gen_now;
            std::expected<sxm::Token, sxm::ParserError> &token_next = *token_gen_now;

            if (token_next->kind == sxm::Token_Kind::FnOpen) {
                auto node_fn = new sxm::PEvalNodeFn {
                    pointer,
                    parent,
                    sxm::ASTNode_Kind::Fn,
                    name,
                    1,
                };

                *pointer = node_fn;

                parent = node_fn;
                pointer = &node_fn->arg_list.back();

                ++token_gen_now;
            } else {
                *pointer = new sxm::PEvalNodeRef {
                    pointer,
                    parent,
                    sxm::ASTNode_Kind::Ref,
                    name
                };
            }

            break;
        }
        case sxm::Token_Kind::NumberInt: {
            *pointer = new sxm::PEvalNodeNum {
                pointer,
                parent,
                sxm::ASTNode_Kind::Num,
                sxm::NumValue::parse_int(token_now->section_start, token_now->section_end),
            };

            ++token_gen_now;

            break;
        }
        case sxm::Token_Kind::NumberReal: {
            *pointer = new sxm::PEvalNodeNum {
                pointer,
                parent,
                sxm::ASTNode_Kind::Num,
                sxm::NumValue::parse_real(token_now->section_start, token_now->section_end),
            };

            ++token_gen_now;

            break;
        }
        case sxm::Token_Kind::PriorityOpen: {
            auto node_priority = new sxm::PEvalNodePriority {
                pointer,
                parent,
                sxm::ASTNode_Kind::Priority,
                nullptr
            };

            {
                *pointer = node_priority;
            }

            {
                parent = node_priority;
                pointer = &node_priority->child;
            }

            ++token_gen_now;

            break;
        }
        case sxm::Token_Kind::PriorityClose: {
            auto search_ptr = *pointer;

            while (true) {
                if (search_ptr->kind == sxm::ASTNode_Kind::Priority) {
                    pointer = search_ptr->slot;
                    parent = search_ptr->parent;

                    break;
                }

                search_ptr = search_ptr->parent;
            }

            ++token_gen_now;

            break;
        }
        case sxm::Token_Kind::FnClose: {
            auto search_ptr = parent;

            while (true) {
                if (search_ptr->kind == sxm::ASTNode_Kind::Fn) {
                    auto node_fn = static_cast<sxm::PEvalNodeFn *>(search_ptr);

                    pointer = node_fn->slot;
                    parent = node_fn->parent;

                    if (node_fn->arg_list.back() == nullptr) {
                        node_fn->arg_list.pop_back();
                    }

                    break;
                }

                search_ptr = search_ptr->parent;
            }

            ++token_gen_now;

            break;
        }
        case sxm::Token_Kind::FnComma: {
            auto search_ptr = (*pointer)->parent;

            while (true) {
                if (search_ptr->kind == sxm::ASTNode_Kind::Fn) {
                    auto node_fn = static_cast<sxm::PEvalNodeFn *>(search_ptr);

                    {
                        sxm::PEvalNode **data_before = node_fn->arg_list.data();

                        node_fn->arg_list.emplace_back();

                        sxm::PEvalNode **data_now = node_fn->arg_list.data();

                        if (data_before != data_now) {
                            // last element is uninitialised
                            sxm::PEvalNode **data_end = data_now + node_fn->arg_list.size() -1;

                            for (; data_now < data_end; ++data_now) {
                                (*data_now)->slot = data_now;
                            }
                        }
                    }

                    pointer = &node_fn->arg_list.back();
                    parent = node_fn;

                    break;
                }

                search_ptr = search_ptr->parent;
            }

            ++token_gen_now;

            break;
        }
        case sxm::Token_Kind::UnaryOp: {
            const auto uop = sxm::uop_let(token_now->section_start);

            auto node_uop = new sxm::PEvalNodeUOp {
                pointer,
                parent,
                sxm::ASTNode_Kind::UOp,
                uop,
                nullptr,
            };

            {
                *pointer = node_uop;
            }

            {
                parent = node_uop;
                pointer = &node_uop->child;
            }

            ++token_gen_now;

            break;
        }
        case sxm::Token_Kind::BinaryOp: {
            auto target = *pointer;

            const sxm::BinaryOp biop = biop_let(token_now->section_start, token_now->section_end);
            const unsigned short biop_priority = biop_priority_let(biop);

            while (target->parent != nullptr && target->parent->kind == sxm::ASTNode_Kind::BiOp) {
                const auto target_parent = static_cast<sxm::PEvalNodeBiOp *>(target->parent);
                const unsigned short target_parent_priority = biop_priority_let(target_parent->operation);

                if (target_parent_priority > biop_priority) {
                    target = target_parent;
                } else {
                    break;
                }
            }

            auto node_biop = new sxm::PEvalNodeBiOp {
                target->slot,
                target->parent,
                sxm::ASTNode_Kind::BiOp,
                biop,
                target,
                nullptr,
            };

            {
                *(target->slot) = node_biop;
                target->parent = node_biop;
                target->slot = &node_biop->left;
            }

            {
                parent = node_biop;
                pointer = &node_biop->right;
            }

            ++token_gen_now;

            break;
        }
        }
    }
}

std::expected<const sxm::NumValue, sxm::EvalError> sxm::eval_pure(sxm::PEvalNode *root, HMap<sxm::NumValue> *val_hmap) {
    sxm::NumValue result {};
    std::vector<StackNode> stack {};

    {
        stack_handle(&stack, root, &result);
    }

    while (stack.size() >= 1) {
        StackNode *snode = stack.data() + stack.size() - 1;

        if (snode->check) {
            switch (snode->ast->kind) {
            case sxm::ASTNode_Kind::Ref: {
                auto noderef = static_cast<sxm::PEvalNodeRef *>(snode->ast);
                auto ref_value = val_hmap->find(noderef->name);

                if (ref_value.has_value()) {
                    *snode->output = ref_value.value();
                } else {
                    return std::unexpected {
                        sxm::EvalError {
                            sxm::EvalError_Kind::VarUndef,
                            "Undefined Variable " + std::string { noderef->name.p_start, noderef->name.p_end }
                        },
                    };
                }

                break;
            }
            case sxm::ASTNode_Kind::UOp: {
                handle_uop(snode, static_cast<sxm::PEvalNodeUOp *>(snode->ast));

                break;
            }
            case sxm::ASTNode_Kind::BiOp: {
                handle_biop(snode, static_cast<sxm::PEvalNodeBiOp *>(snode->ast), val_hmap);

                break;
            }
            case sxm::ASTNode_Kind::Fn: {
                auto nodefn = static_cast<sxm::PEvalNodeFn *>(snode->ast);
                auto nodefn_numfn_o = fn_hmap.find(nodefn->name);

                if (!nodefn_numfn_o.has_value()) {
                    return std::unexpected {
                        sxm::EvalError {
                            sxm::EvalError_Kind::FnUndef,
                            "Undefined Function " + std::string { nodefn->name.p_start, nodefn->name.p_end }
                        },
                    };
                }

                auto nodefn_numfn = nodefn_numfn_o.value();
                auto next_o = nodefn_numfn({ snode->value_list, snode->value_list + nodefn->arg_list.size() });

                if (!next_o.has_value()) {
                    return std::unexpected {
                        sxm::EvalError {
                            sxm::EvalError_Kind::FnEval,
                            "Function Evaluation Error in Function " + std::string { nodefn->name.p_start, nodefn->name.p_end }
                        },
                    };
                }

                *snode->output = next_o.value();

                break;
            }
            default: {
                return std::unexpected {
                    sxm::EvalError {
                        sxm::EvalError_Kind::NodeUnexp,
                        "Unexpected Node of Kind " + sxm::astnode_kind_tostr(snode->ast->kind),
                    },
                };
            }
            }

            stack.pop_back();

            continue;
        }

        snode->check = true;

        switch (snode->ast->kind) {
        case sxm::ASTNode_Kind::Num:
        case sxm::ASTNode_Kind::Ref:
        case sxm::ASTNode_Kind::Priority: {
            std::unreachable();
        }
        case sxm::ASTNode_Kind::UOp: {
            auto nodeop = static_cast<sxm::PEvalNodeUOp *>(snode->ast);

            snode->value_list = new NumValue[1];

            stack_handle(&stack, nodeop->child, snode->value_list);

            break;
        }
        case sxm::ASTNode_Kind::BiOp: {
            auto nodeop = static_cast<sxm::PEvalNodeBiOp *>(snode->ast);

            NumValue *value_list = new NumValue[2];

            snode->value_list = value_list;

            stack_handle(&stack, nodeop->right, value_list + 1);

            if (!(nodeop->operation == sxm::BinaryOp::Assign && nodeop->left->kind == sxm::ASTNode_Kind::Ref)) {
                stack_handle(&stack, nodeop->left, value_list);
            }

            break;
        }
        case sxm::ASTNode_Kind::Fn: {
            auto nodefn = static_cast<sxm::PEvalNodeFn *>(snode->ast);
            std::size_t arg_list_size = nodefn->arg_list.size();

            if (arg_list_size >= 1) {
                NumValue *value_list = new NumValue[arg_list_size];

                snode->value_list = value_list;

                for (std::size_t i = arg_list_size; (i--) >= 1;) {
                    stack_handle(&stack, nodefn->arg_list[i], value_list + i);
                }
            }

            break;
        }
        }
    }

    return result;
}
