#include "algo/list/range.hpp"
#include "algo/search_map/h_const.hpp"
#include "sxm/eval/flat.hpp"
#include "sxm/eval/linked.hpp"
#include "sxm/ast/numvalue.hpp"
#include <cstddef>

#pragma once

namespace sxm::numfn {

using Validator = bool (*)(std::size_t arg_size);
using FnRaw = sxm::NumValue (*)(Range<sxm::NumValue> arg_list);
using FnFEval = sxm::NumValue (*)(Range<sxm::FEvalNodeNum *> arg_list);
using FnLEval = sxm::NumValue (*)(Range<sxm::LEvalNodeNum *> arg_list);

struct Def {
    FnRaw fn_raw;
    FnFEval fn_feval;
    FnLEval fn_leval;
    Validator validator;
};

sxm::NumValue int_fn_raw(Range<sxm::NumValue> arg_list);
sxm::NumValue int_fn_feval(Range<sxm::FEvalNodeNum *> arg_list);
sxm::NumValue int_fn_leval(Range<sxm::LEvalNodeNum *> arg_list);
bool int_validator(std::size_t arg_size);
constexpr Def int_def { int_fn_raw, int_fn_feval, int_fn_leval, int_validator, };

sxm::NumValue if_fn_raw(Range<sxm::NumValue> arg_list);
sxm::NumValue if_fn_feval(Range<sxm::FEvalNodeNum *> arg_list);
sxm::NumValue if_fn_leval(Range<sxm::LEvalNodeNum *> arg_list);
bool if_validator(std::size_t arg_size);
constexpr Def if_def { if_fn_raw, if_fn_feval, if_fn_leval, if_validator, };

sxm::NumValue ife_fn_raw(Range<sxm::NumValue> arg_list);
sxm::NumValue ife_fn_feval(Range<sxm::FEvalNodeNum *> arg_list);
sxm::NumValue ife_fn_leval(Range<sxm::LEvalNodeNum *> arg_list);
bool ife_validator(std::size_t arg_size);
constexpr Def ife_def { ife_fn_raw, ife_fn_feval, ife_fn_leval, ife_validator, };

sxm::NumValue sin_fn_raw(Range<sxm::NumValue> arg_list);
sxm::NumValue sin_fn_feval(Range<sxm::FEvalNodeNum *> arg_list);
sxm::NumValue sin_fn_leval(Range<sxm::LEvalNodeNum *> arg_list);
bool sin_validator(std::size_t arg_size);
constexpr Def sin_def { sin_fn_raw, sin_fn_feval, sin_fn_leval, sin_validator, };

sxm::NumValue cos_fn_raw(Range<sxm::NumValue> arg_list);
sxm::NumValue cos_fn_feval(Range<sxm::FEvalNodeNum *> arg_list);
sxm::NumValue cos_fn_leval(Range<sxm::LEvalNodeNum *> arg_list);
bool cos_validator(std::size_t arg_size);
constexpr Def cos_def { cos_fn_raw, cos_fn_feval, cos_fn_leval, cos_validator, };

sxm::NumValue tan_fn_raw(Range<sxm::NumValue> arg_list);
sxm::NumValue tan_fn_feval(Range<sxm::FEvalNodeNum *> arg_list);
sxm::NumValue tan_fn_leval(Range<sxm::LEvalNodeNum *> arg_list);
bool tan_validator(std::size_t arg_size);
constexpr Def tan_def { tan_fn_raw, tan_fn_feval, tan_fn_leval, tan_validator, };

sxm::NumValue ctan_fn_raw(Range<sxm::NumValue> arg_list);
sxm::NumValue ctan_fn_feval(Range<sxm::FEvalNodeNum *> arg_list);
sxm::NumValue ctan_fn_leval(Range<sxm::LEvalNodeNum *> arg_list);
bool ctan_validator(std::size_t arg_size);
constexpr Def ctan_def { ctan_fn_raw, ctan_fn_feval, ctan_fn_leval, ctan_validator, };

sxm::NumValue ln_fn_raw(Range<sxm::NumValue> arg_list);
sxm::NumValue ln_fn_feval(Range<sxm::FEvalNodeNum *> arg_list);
sxm::NumValue ln_fn_leval(Range<sxm::LEvalNodeNum *> arg_list);
bool ln_validator(std::size_t arg_size);
constexpr Def ln_def { ln_fn_raw, ln_fn_feval, ln_fn_leval, ln_validator, };

sxm::NumValue log_fn_raw(Range<sxm::NumValue> arg_list);
sxm::NumValue log_fn_feval(Range<sxm::FEvalNodeNum *> arg_list);
sxm::NumValue log_fn_leval(Range<sxm::LEvalNodeNum *> arg_list);
bool log_validator(std::size_t arg_size);
constexpr Def log_def { log_fn_raw, log_fn_feval, log_fn_leval, log_validator, };

sxm::NumValue log2_fn_raw(Range<sxm::NumValue> arg_list);
sxm::NumValue log2_fn_feval(Range<sxm::FEvalNodeNum *> arg_list);
sxm::NumValue log2_fn_leval(Range<sxm::LEvalNodeNum *> arg_list);
bool log2_validator(std::size_t arg_size);
constexpr Def log2_def { log2_fn_raw, log2_fn_feval, log2_fn_leval, log2_validator, };

sxm::NumValue log10_fn_raw(Range<sxm::NumValue> arg_list);
sxm::NumValue log10_fn_feval(Range<sxm::FEvalNodeNum *> arg_list);
sxm::NumValue log10_fn_leval(Range<sxm::LEvalNodeNum *> arg_list);
bool log10_validator(std::size_t arg_size);
constexpr Def log10_def { log10_fn_raw, log10_fn_feval, log10_fn_leval, log10_validator, };

sxm::NumValue b_fn_raw(Range<sxm::NumValue> arg_list);
sxm::NumValue b_fn_feval(Range<sxm::FEvalNodeNum *> arg_list);
sxm::NumValue b_fn_leval(Range<sxm::LEvalNodeNum *> arg_list);
bool b_validator(std::size_t arg_size);
constexpr Def b_def { b_fn_raw, b_fn_feval, b_fn_leval, b_validator, };

constexpr HMapConst<64, 12, Def> hmap {
    std::array<HMapConst_HNode<Def>, 12> {
        HMapConst_HNode<Def>{ Range<const char>{ "b" }, sxm::numfn::b_def },
        HMapConst_HNode<Def>{ Range<const char>{ "int" }, sxm::numfn::int_def },

        HMapConst_HNode<Def>{ Range<const char>{ "if" }, sxm::numfn::if_def },
        HMapConst_HNode<Def>{ Range<const char>{ "ife" }, sxm::numfn::ife_def },

        HMapConst_HNode<Def>{ Range<const char>{ "sin" }, sxm::numfn::sin_def },
        HMapConst_HNode<Def>{ Range<const char>{ "cos" }, sxm::numfn::cos_def },
        HMapConst_HNode<Def>{ Range<const char>{ "tan" }, sxm::numfn::tan_def },
        HMapConst_HNode<Def>{ Range<const char>{ "ctan" }, sxm::numfn::ctan_def },

        HMapConst_HNode<Def>{ Range<const char>{ "ln" }, sxm::numfn::ln_def },
        HMapConst_HNode<Def>{ Range<const char>{ "log" }, sxm::numfn::log_def },
        HMapConst_HNode<Def>{ Range<const char>{ "log2" }, sxm::numfn::log2_def },
        HMapConst_HNode<Def>{ Range<const char>{ "log10" }, sxm::numfn::log10_def },
    }
};

}
