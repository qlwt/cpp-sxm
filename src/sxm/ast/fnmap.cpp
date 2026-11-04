#include "algo/list/range.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/ast/fnmap.hpp"
#include <cmath>
#include <cstddef>

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue proj(Arg *ptr) {
    if constexpr (Member == nullptr) {
        return *ptr;
    } else {
        return *ptr->*Member;
    }
}

}

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue int_proj(Range<Arg> arg_list) {
    double real = std::trunc(proj<Arg, Member>(arg_list.p_start).real);

    return sxm::NumValue { real, };
}

}

sxm::NumValue sxm::numfn::int_fn_raw(Range<sxm::NumValue> arg_list) {
    return int_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::int_fn_feval(Range<sxm::FEvalNodeNum *> arg_list) {
    return int_proj<sxm::FEvalNodeNum *, &sxm::FEvalNodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::int_fn_leval(Range<sxm::LEvalNodeNum *> arg_list) {
    return int_proj<sxm::LEvalNodeNum *, &sxm::LEvalNodeNum::value>(arg_list);
}

bool sxm::numfn::int_validator(std::size_t arg_size) {
    return arg_size == 1;
}

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue if_proj(Range<Arg> arg_list) {
    for (Arg *arg_now = arg_list.p_start; arg_now < arg_list.p_end; ++arg_now) {
        sxm::NumValue arg_now_value = proj<Arg, Member>(arg_now);

        if (std::abs(arg_now_value.real) > sxm::NumValue::TOLERANCE_ABS) {
            return arg_now_value;
        }
    }

    return proj<Arg, Member>(arg_list.p_end - 1);
}

}

sxm::NumValue sxm::numfn::if_fn_raw(Range<sxm::NumValue> arg_list) {
    return if_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::if_fn_feval(Range<sxm::FEvalNodeNum *> arg_list) {
    return if_proj<sxm::FEvalNodeNum *, &sxm::FEvalNodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::if_fn_leval(Range<sxm::LEvalNodeNum *> arg_list) {
    return if_proj<sxm::LEvalNodeNum *, &sxm::LEvalNodeNum::value>(arg_list);
}

bool sxm::numfn::if_validator(std::size_t arg_size) {
    return arg_size >= 1;
}

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue ife_proj(Range<Arg> arg_list) {
    if (std::abs(proj<Arg, Member>(arg_list.p_start).real) > sxm::NumValue::TOLERANCE_ABS) {
        return proj<Arg, Member>(arg_list.p_start + 1);
    }

    return proj<Arg, Member>(arg_list.p_start + 2);
}

}

sxm::NumValue sxm::numfn::ife_fn_raw(Range<sxm::NumValue> arg_list) {
    return ife_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::ife_fn_feval(Range<sxm::FEvalNodeNum *> arg_list) {
    return ife_proj<sxm::FEvalNodeNum *, &sxm::FEvalNodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::ife_fn_leval(Range<sxm::LEvalNodeNum *> arg_list) {
    return ife_proj<sxm::LEvalNodeNum *, &sxm::LEvalNodeNum::value>(arg_list);
}

bool sxm::numfn::ife_validator(std::size_t arg_size) {
    return arg_size == 3;
}

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue ctan_proj(Range<Arg> arg_list) {
    return sxm::NumValue {
        1 / std::tan(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::ctan_fn_raw(Range<sxm::NumValue> arg_list) {
    return ctan_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::ctan_fn_feval(Range<sxm::FEvalNodeNum *> arg_list) {
    return ctan_proj<sxm::FEvalNodeNum *, &sxm::FEvalNodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::ctan_fn_leval(Range<sxm::LEvalNodeNum *> arg_list) {
    return ctan_proj<sxm::LEvalNodeNum *, &sxm::LEvalNodeNum::value>(arg_list);
}

bool sxm::numfn::ctan_validator(std::size_t arg_size) {
    return arg_size == 1;
}

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue tan_proj(Range<Arg> arg_list) {
    return sxm::NumValue {
        std::tan(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::tan_fn_raw(Range<sxm::NumValue> arg_list) {
    return tan_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::tan_fn_feval(Range<sxm::FEvalNodeNum *> arg_list) {
    return tan_proj<sxm::FEvalNodeNum *, &sxm::FEvalNodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::tan_fn_leval(Range<sxm::LEvalNodeNum *> arg_list) {
    return tan_proj<sxm::LEvalNodeNum *, &sxm::LEvalNodeNum::value>(arg_list);
}

bool sxm::numfn::tan_validator(std::size_t arg_size) {
    return arg_size == 1;
}

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue cos_proj(Range<Arg> arg_list) {
    return sxm::NumValue {
        std::cos(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::cos_fn_raw(Range<sxm::NumValue> arg_list) {
    return cos_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::cos_fn_feval(Range<sxm::FEvalNodeNum *> arg_list) {
    return cos_proj<sxm::FEvalNodeNum *, &sxm::FEvalNodeNum::value>(arg_list);
};

sxm::NumValue sxm::numfn::cos_fn_leval(Range<sxm::LEvalNodeNum *> arg_list) {
    return cos_proj<sxm::LEvalNodeNum *, &sxm::LEvalNodeNum::value>(arg_list);
};

bool sxm::numfn::cos_validator(std::size_t arg_size) {
    return arg_size == 1;
}

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue sin_proj(Range<Arg> arg_list) {
    return sxm::NumValue {
        std::sin(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::sin_fn_raw(Range<sxm::NumValue> arg_list) {
    return sin_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::sin_fn_feval(Range<sxm::FEvalNodeNum *> arg_list) {
    return sin_proj<sxm::FEvalNodeNum *, &sxm::FEvalNodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::sin_fn_leval(Range<sxm::LEvalNodeNum *> arg_list) {
    return sin_proj<sxm::LEvalNodeNum *, &sxm::LEvalNodeNum::value>(arg_list);
}

bool sxm::numfn::sin_validator(std::size_t arg_size) {
    return arg_size == 1;
}

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue log_proj(Range<Arg> arg_list) {
    return sxm::NumValue {
        (
            std::log(proj<Arg, Member>(arg_list.p_start + 1).real)
            / std::log(proj<Arg, Member>(arg_list.p_start).real)
        ),
    };
}

}

sxm::NumValue sxm::numfn::log_fn_raw(Range<sxm::NumValue> arg_list) {
    return log_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::log_fn_feval(Range<sxm::FEvalNodeNum *> arg_list) {
    return log_proj<sxm::FEvalNodeNum *, &sxm::FEvalNodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::log_fn_leval(Range<sxm::LEvalNodeNum *> arg_list) {
    return log_proj<sxm::LEvalNodeNum *, &sxm::LEvalNodeNum::value>(arg_list);
}

bool sxm::numfn::log_validator(std::size_t arg_size) {
    return arg_size == 2;
}

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue ln_proj(Range<Arg> arg_list) {
    return sxm::NumValue {
        std::log(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::ln_fn_raw(Range<sxm::NumValue> arg_list) {
    return ln_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::ln_fn_feval(Range<sxm::FEvalNodeNum *> arg_list) {
    return ln_proj<sxm::FEvalNodeNum *, &sxm::FEvalNodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::ln_fn_leval(Range<sxm::LEvalNodeNum *> arg_list) {
    return ln_proj<sxm::LEvalNodeNum *, &sxm::LEvalNodeNum::value>(arg_list);
}

bool sxm::numfn::ln_validator(std::size_t arg_size) {
    return arg_size == 1;
}

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue log10_proj(Range<Arg> arg_list) {
    return sxm::NumValue {
        std::log10(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::log10_fn_raw(Range<sxm::NumValue> arg_list) {
    return log10_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::log10_fn_feval(Range<sxm::FEvalNodeNum *> arg_list) {
    return log10_proj<sxm::FEvalNodeNum *, &sxm::FEvalNodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::log10_fn_leval(Range<sxm::LEvalNodeNum *> arg_list) {
    return log10_proj<sxm::LEvalNodeNum *, &sxm::LEvalNodeNum::value>(arg_list);
}

bool sxm::numfn::log10_validator(std::size_t arg_size) {
    return arg_size == 1;
}

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue log2_proj(Range<Arg> arg_list) {
    return sxm::NumValue {
        std::log2(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::log2_fn_raw(Range<sxm::NumValue> arg_list) {
    return log2_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::log2_fn_feval(Range<sxm::FEvalNodeNum *> arg_list) {
    return log2_proj<sxm::FEvalNodeNum *, &sxm::FEvalNodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::log2_fn_leval(Range<sxm::LEvalNodeNum *> arg_list) {
    return log2_proj<sxm::LEvalNodeNum *, &sxm::LEvalNodeNum::value>(arg_list);
}

bool sxm::numfn::log2_validator(std::size_t arg_size) {
    return arg_size == 1;
}

sxm::NumValue sxm::numfn::b_fn_raw(Range<sxm::NumValue>) {
    return NumValue { 0, };
}

sxm::NumValue sxm::numfn::b_fn_feval(Range<sxm::FEvalNodeNum *>) {
    return NumValue { 0, };
}

sxm::NumValue sxm::numfn::b_fn_leval(Range<sxm::LEvalNodeNum *>) {
    return NumValue { 0, };
}

bool sxm::numfn::b_validator(std::size_t) {
    return true;
}
