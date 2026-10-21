#include "sxm/debug/valid_let.hpp"

namespace {

bool valid_let_raw(const sxm::PEvalNode *root, const sxm::PEvalNode *parent, sxm::PEvalNode *const *slot) {
    if (root->parent != parent) {
        return false;
    }

    if (root->slot != slot) {
        return false;
    }

    switch (root->kind) {
    case sxm::ASTNode_Kind::Ref:
    case sxm::ASTNode_Kind::Num: {
        return true;
    }
    case sxm::ASTNode_Kind::UOp: {
        const auto lnode = static_cast<const sxm::PEvalNodeUOp *>(root);

        return valid_let_raw(lnode->child, lnode, &lnode->child);
    }
    case sxm::ASTNode_Kind::BiOp: {
        const auto lnode = static_cast<const sxm::PEvalNodeBiOp *>(root);

        return valid_let_raw(lnode->left, lnode, &lnode->left) && valid_let_raw(lnode->right, lnode, &lnode->right);
    }
    case sxm::ASTNode_Kind::Fn: {
        const auto lnode = static_cast<const sxm::PEvalNodeFn *>(root);

        sxm::PEvalNode *const *list_now = lnode->arg_list.data();
        sxm::PEvalNode *const *list_end = list_now + lnode->arg_list.size();

        for (; list_now < list_end; ++list_now) {
            if (!valid_let_raw(*list_now, lnode, list_now)) {
                return false;
            }
        }


        return true;
    }
    case sxm::ASTNode_Kind::Priority: {
        const auto lnode = static_cast<const sxm::PEvalNodePriority *>(root);

        return valid_let_raw(lnode->child, lnode, &lnode->child);
    }
    }

    return false;
}

}

bool sxm::valid_let(const sxm::PEvalNode *root) {
    return valid_let_raw(root, nullptr, nullptr);
}
