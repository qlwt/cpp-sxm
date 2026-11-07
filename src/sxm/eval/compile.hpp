#include "algo/list/range.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/ast/fnmap.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/ast/uop.hpp"
#include <cstddef>
#include <expected>
#include <vector>

#pragma once

namespace sxm {

using CEvalError = std::variant<sxm::ParserError, sxm::EvalError>;

struct CEvalOffset {
    std::size_t raw { 0 };

    inline CEvalOffset() noexcept {}

    inline CEvalOffset(std::size_t raw) noexcept
        : raw(raw) { }

    inline CEvalOffset(std::size_t raw, bool flag_const) noexcept
        : raw(raw | ((std::size_t)flag_const << 63)) {}

    inline bool flag_const() const noexcept {
        return this->raw >> 63;
    };

    inline bool flag_state() const noexcept {
        return !this->flag_const();
    };

    inline std::size_t get() const noexcept {
        static constexpr std::size_t flag_const = 1lu << 63;
        static constexpr std::size_t nflag_const = ~flag_const;

        return this->raw & nflag_const;
    };

    inline sxm::NumValue *resolve(sxm::NumValue *const_list, sxm::NumValue *state_list) const noexcept {
        if (this->flag_const()) {
            return const_list + this->get();
        }

        return state_list + this->raw;
    }
};

union CEvalInstr_Ptr {
    std::size_t length;
    CEvalOffset offset;
    numfn::FnRaw fn;
    NumValue *value;
};

enum class CEvalInstr_Kind {
    Ret,
    Call,
    ArgPush,

    Set,
    Neg,

    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Pow,
    Eq,
    UEq,
    GTE,
    LTE,
    LT,
    GT,
    Assign,
};

struct CEvalInstr {
    CEvalInstr_Kind kind;
    CEvalInstr_Ptr left;
    CEvalInstr_Ptr right;
    CEvalInstr_Ptr output;
};

struct CEvalUnit {
    Range<NumValue> arg_list;
    Range<NumValue> state_list;
    Range<CEvalInstr> inst_list;

    CEvalUnit() = default;
    CEvalUnit(Range<NumValue> arg_list, Range<NumValue> state_list, Range<CEvalInstr> inst_list)
        : arg_list(arg_list), state_list(state_list), inst_list(inst_list) {};

    CEvalUnit(CEvalUnit &src) = delete;
    CEvalUnit operator=(CEvalUnit &src) = delete;

    CEvalUnit(CEvalUnit &&src) noexcept {
        this->arg_list = src.arg_list;
        this->inst_list = src.inst_list;
        this->state_list = src.state_list;

        src.state_list.p_start = nullptr;
        src.state_list.p_end = nullptr;

        src.arg_list.p_start = nullptr;
        src.arg_list.p_end = nullptr;

        src.inst_list.p_start = nullptr;
        src.inst_list.p_end = nullptr;
    };

    CEvalUnit &operator=(CEvalUnit &&src) noexcept {
        this->state_list = src.state_list;
        this->arg_list = src.arg_list;
        this->inst_list = src.inst_list;

        src.state_list.p_start = nullptr;
        src.state_list.p_end = nullptr;

        src.arg_list.p_start = nullptr;
        src.arg_list.p_end = nullptr;

        src.inst_list.p_start = nullptr;
        src.inst_list.p_end = nullptr;

        return *this;
    };

    ~CEvalUnit() {
        std::free(this->inst_list.p_start);
    }

    sxm::NumValue eval() noexcept;
};

struct CEvalDeclaration {
    std::size_t arg_size;
    std::size_t state_size;

    Range<NumValue> const_list;
    Range<CEvalInstr> inst_list;

    CEvalDeclaration() = default;
    CEvalDeclaration(std::size_t arg_size, std::size_t state_size, Range<NumValue> const_list, Range<CEvalInstr> inst_list)
        : arg_size(arg_size), state_size(state_size), const_list(const_list), inst_list(inst_list) {};

    CEvalDeclaration(CEvalDeclaration &src) = delete;
    CEvalDeclaration operator=(CEvalDeclaration &src) = delete;

    CEvalDeclaration(CEvalDeclaration &&src) noexcept {
        this->inst_list = src.inst_list;
        this->const_list = src.const_list;
        this->arg_size = src.arg_size;
        this->state_size = src.state_size;

        src.const_list.p_start = nullptr;
        src.const_list.p_end = nullptr;

        src.inst_list.p_start = nullptr;
        src.inst_list.p_end = nullptr;
    };

    CEvalDeclaration &operator=(CEvalDeclaration &&src) noexcept {
        this->const_list = src.const_list;
        this->inst_list = src.inst_list;
        this->arg_size = src.arg_size;
        this->state_size = src.state_size;

        src.const_list.p_start = nullptr;
        src.const_list.p_end = nullptr;
        src.inst_list.p_start = nullptr;
        src.inst_list.p_end = nullptr;

        return *this;
    };

    ~CEvalDeclaration() {
        std::free(this->const_list.p_start);
        std::free(this->inst_list.p_start);
    }

    sxm::CEvalUnit init() const noexcept;
    sxm::NumValue eval(Range<sxm::NumValue> sxmvar_local) const noexcept;
};

struct CEvalNode {
    ASTNode_Kind kind;
    CEvalNode **slot;
    CEvalNode *parent;

    CEvalNode() = default;
    CEvalNode(ASTNode_Kind kind, CEvalNode **slot, CEvalNode *parent)
        : kind(kind), slot(slot), parent(parent) {};

    CEvalNode(const CEvalNode &src) = delete;
    CEvalNode& operator=(const CEvalNode &src) = delete;

    CEvalNode(CEvalNode &&src) = default;
    CEvalNode& operator=(CEvalNode &&src) = default;

    virtual ~CEvalNode() = default;
};

struct CEvalNodeNum : CEvalNode {
    CEvalOffset offset;
    bool flag_persistent;

    CEvalNodeNum() = default;
    CEvalNodeNum(CEvalNode **slot, CEvalNode *parent, CEvalOffset offset, bool flag_persistent)
        : CEvalNode(ASTNode_Kind::Num, slot, parent), offset(offset), flag_persistent(flag_persistent) {};

    ~CEvalNodeNum() = default;
};

struct CEvalNodeRef : CEvalNode {
    Range<const char> name;

    CEvalNodeRef(CEvalNode **slot, CEvalNode *parent, Range<const char> name)
        : CEvalNode(ASTNode_Kind::Ref, slot, parent), name(name) {};

    ~CEvalNodeRef() = default;
};

struct CEvalNodePriority : CEvalNode {
    CEvalNode *child;

    CEvalNodePriority(CEvalNode **slot, CEvalNode *parent, CEvalNode *child)
        : CEvalNode(ASTNode_Kind::Priority, slot, parent), child(child) {};

    ~CEvalNodePriority();
};

struct CEvalNodeBiOp : CEvalNode {
    sxm::BinaryOp operation;
    CEvalNode *left;
    CEvalNode *right;

    CEvalNodeBiOp() = default;
    CEvalNodeBiOp(CEvalNode **slot, CEvalNode *parent, BinaryOp operation, CEvalNode *left, CEvalNode *right)
        : CEvalNode(ASTNode_Kind::BiOp, slot, parent), operation(operation), left(left), right(right) {};

    ~CEvalNodeBiOp();
};

struct CEvalNodeUOp : CEvalNode {
    sxm::UnaryOp operation;
    CEvalNode *child;

    CEvalNodeUOp() = default;
    CEvalNodeUOp(CEvalNode **slot, CEvalNode *parent, UnaryOp operation, CEvalNode *child)
        : CEvalNode(ASTNode_Kind::UOp, slot, parent), operation(operation), child(child) {};

    ~CEvalNodeUOp();
};

struct CEvalNodeFn : CEvalNode {
    Range<const char> name;
    std::vector<CEvalNode *> arg_list;

    CEvalNodeFn(CEvalNode **slot, CEvalNode *parent, Range<const char> name, std::vector<CEvalNode *> arg_list)
        : CEvalNode(ASTNode_Kind::Fn, slot, parent), name(name), arg_list(arg_list) {};

    ~CEvalNodeFn();
};

struct CEval_CompileBuild_Config {
    IMap<sxm::NumValue> *sxmvar_const;
    IMap<sxm::CEvalOffset> *sxmvar_state;
    std::size_t sxmvar_state_size;
};

std::expected<CEvalDeclaration, CEvalError> eval_compile_build(const char *src, const CEval_CompileBuild_Config &config);

}
