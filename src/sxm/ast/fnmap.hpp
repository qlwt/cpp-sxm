#include "algo/list/range.hpp"
#include "algo/search_map/h_const.hpp"
#include "sxm/eval/flat.hpp"
#include "sxm/eval/linked.hpp"
#include "sxm/ast/numvalue.hpp"
#include <optional>

#pragma once

namespace sxm {

using NumFn = std::optional<sxm::NumValue> (*)(Range<sxm::NumValue> arg_list);
using NumFnFEval = std::optional<sxm::NumValue> (*)(Range<sxm::FEvalNodeNum *> arg_list);
using NumFnLEval = std::optional<sxm::NumValue> (*)(Range<sxm::LEvalNodeNum *> arg_list);

namespace numfn {

std::optional<sxm::NumValue> toint(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_toint(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_toint(Range<sxm::LEvalNodeNum *> arg_list);

std::optional<sxm::NumValue> bif(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_bif(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_bif(Range<sxm::LEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> bife(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_bife(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_bife(Range<sxm::LEvalNodeNum *> arg_list);

std::optional<sxm::NumValue> sin(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_sin(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_sin(Range<sxm::LEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> cos(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_cos(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_cos(Range<sxm::LEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> tan(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_tan(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_tan(Range<sxm::LEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> ctan(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_ctan(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_ctan(Range<sxm::LEvalNodeNum *> arg_list);

std::optional<sxm::NumValue> ln(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_ln(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_ln(Range<sxm::LEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> log(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_log(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_log(Range<sxm::LEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> log2(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_log2(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_log2(Range<sxm::LEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> log10(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_log10(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_log10(Range<sxm::LEvalNodeNum *> arg_list);

std::optional<sxm::NumValue> toreal(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_toreal(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_toreal(Range<sxm::LEvalNodeNum *> arg_list);

std::optional<sxm::NumValue> b(Range<sxm::NumValue> arg_list);
std::optional<sxm::NumValue> feval_b(Range<sxm::FEvalNodeNum *> arg_list);
std::optional<sxm::NumValue> leval_b(Range<sxm::LEvalNodeNum *> arg_list);

}

constexpr HMapConst<64, 15, sxm::NumFn> fn_hmap {
    std::array<HMapConst_HNode<sxm::NumFn>, 15> {
        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "b" }, sxm::numfn::b },

        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "int" }, sxm::numfn::toint },
        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "real" }, sxm::numfn::toreal },

        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "if" }, sxm::numfn::bif },
        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "ife" }, sxm::numfn::bife },

        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "sin" }, sxm::numfn::sin },
        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "cos" }, sxm::numfn::cos },
        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "tan" }, sxm::numfn::tan },
        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "ctan" }, sxm::numfn::ctan },

        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "ln" }, sxm::numfn::ln },
        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "log" }, sxm::numfn::log },
        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "log2" }, sxm::numfn::log2 },
        HMapConst_HNode<sxm::NumFn>{ Range<const char>{ "log10" }, sxm::numfn::log10 },
    }
};

constexpr HMapConst<64, 15, sxm::NumFnFEval> fn_hmap_feval {
    std::array<HMapConst_HNode<sxm::NumFnFEval>, 15> {
        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "b" }, sxm::numfn::feval_b },

        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "int" }, sxm::numfn::feval_toint },
        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "real" }, sxm::numfn::feval_toreal },

        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "if" }, sxm::numfn::feval_bif },
        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "ife" }, sxm::numfn::feval_bife },

        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "sin" }, sxm::numfn::feval_sin },
        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "cos" }, sxm::numfn::feval_cos },
        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "tan" }, sxm::numfn::feval_tan },
        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "ctan" }, sxm::numfn::feval_ctan },

        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "ln" }, sxm::numfn::feval_ln },
        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "log" }, sxm::numfn::feval_log },
        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "log2" }, sxm::numfn::feval_log2 },
        HMapConst_HNode<sxm::NumFnFEval>{ Range<const char>{ "log10" }, sxm::numfn::feval_log10 },
    }
};

constexpr HMapConst<64, 15, sxm::NumFnLEval> fn_hmap_leval {
    std::array<HMapConst_HNode<sxm::NumFnLEval>, 15> {
        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "b" }, sxm::numfn::leval_b },

        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "int" }, sxm::numfn::leval_toint },
        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "real" }, sxm::numfn::leval_toreal },

        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "if" }, sxm::numfn::leval_bif },
        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "ife" }, sxm::numfn::leval_bife },

        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "sin" }, sxm::numfn::leval_sin },
        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "cos" }, sxm::numfn::leval_cos },
        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "tan" }, sxm::numfn::leval_tan },
        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "ctan" }, sxm::numfn::leval_ctan },

        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "ln" }, sxm::numfn::leval_ln },
        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "log" }, sxm::numfn::leval_log },
        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "log2" }, sxm::numfn::leval_log2 },
        HMapConst_HNode<sxm::NumFnLEval>{ Range<const char>{ "log10" }, sxm::numfn::leval_log10 },
    }
};

}
