#include "sxm/debug/print_ast.hpp"
#include "sxm/debug/tostr.hpp"
#include <cstdio>

namespace {

void print_raw(const sxm::PEvalNode *root, unsigned int space) {
    for (unsigned int i = 0 ; i < space; ++i) {
        printf("\t");
    }

    switch (root->kind) {
    case sxm::ASTNode_Kind::Ref: {
        const auto lnode = static_cast<const sxm::PEvalNodeRef *>(root);

        printf("ref \"%.*s\"", (int)lnode->name.size(), lnode->name.p_start);

        break;
    }
    case sxm::ASTNode_Kind::Num: {
        const auto lnode = static_cast<const sxm::PEvalNodeNum *>(root);

        printf("num: %f", lnode->value.real);

        break;
    }
    case sxm::ASTNode_Kind::UOp: {
        const auto lnode = static_cast<const sxm::PEvalNodeUOp *>(root);

        printf("uop \"%s\": (\n", sxm::uop_tostr(lnode->operation).data());

        print_raw(lnode->child, space + 1);

        printf("\n");

        for (unsigned int i = 0 ; i < space; ++i) {
            printf("\t");
        }

        printf(")");

        break;
    }
    case sxm::ASTNode_Kind::BiOp: {
        const auto lnode = static_cast<const sxm::PEvalNodeBiOp *>(root);

        printf("biop \"%s\": (\n", sxm::biop_tostr(lnode->operation).data());

        print_raw(lnode->left, space + 1);

        printf(",\n");

        print_raw(lnode->right, space + 1);

        printf("\n");

        for (unsigned int i = 0 ; i < space; ++i) {
            printf("\t");
        }

        printf(")");

        break;
    }
    case sxm::ASTNode_Kind::Fn: {
        const auto lnode = static_cast<const sxm::PEvalNodeFn *>(root);

        printf("fn(%ld) \"%.*s\"\n", lnode->arg_list.size(), (int)lnode->name.size(), lnode->name.p_start);

        for (sxm::PEvalNode *arg : lnode->arg_list) {
            print_raw(arg, space + 1);

            printf(",\n");
        }

        for (unsigned int i = 0 ; i < space; ++i) {
            printf("\t");
        }

        printf(")");

        break;
    }
    case sxm::ASTNode_Kind::Priority: {
        const auto lnode = static_cast<const sxm::PEvalNodePriority *>(root);

        printf("priority: (\n");

        print_raw(lnode->child, space + 1);

        printf("\n");

        for (unsigned int i = 0 ; i < space; ++i) {
            printf("\t");
        }

        printf(")");

        break;
    }
    }
}

}

void sxm::print_ast(const sxm::PEvalNode *root) {
    print_raw(root, 0);

    std::printf("\n");
}
