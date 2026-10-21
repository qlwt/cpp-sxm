#include "algo/list/range.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/ast/fnmap.hpp"
#include <cmath>
#include <optional>

namespace {

template <typename Arg, auto Member>
inline std::optional<sxm::NumValue> proj_toint(Range<Arg *> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    double real = std::trunc((*arg_list.p_start->*Member).real);

    return sxm::NumValue { real, };
}

}

std::optional<sxm::NumValue> sxm::numfn::toint(Range<sxm::NumValue> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    double real = std::trunc(arg_list.p_start->real);

    return sxm::NumValue { real, };
}

std::optional<sxm::NumValue> sxm::numfn::feval_toint(Range<sxm::FEvalNodeNum *> arg_list) {
    return proj_toint<sxm::FEvalNodeNum, &sxm::FEvalNodeNum::value>(arg_list);
}

std::optional<sxm::NumValue> sxm::numfn::leval_toint(Range<sxm::LEvalNodeNum *> arg_list) {
    return proj_toint<sxm::LEvalNodeNum, &sxm::LEvalNodeNum::value>(arg_list);
}

namespace {

template <typename Arg, auto Member>
inline std::optional<sxm::NumValue> proj_toreal(Range<Arg *> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue { .real = (*arg_list.p_start->*Member).real, };
}

}

std::optional<sxm::NumValue> sxm::numfn::toreal(Range<sxm::NumValue> arg_list) {
    return sxm::NumValue { .real = arg_list.p_start->real, };
}

std::optional<sxm::NumValue> sxm::numfn::feval_toreal(Range<sxm::FEvalNodeNum *> arg_list) {
    return proj_toreal<sxm::FEvalNodeNum, &sxm::FEvalNodeNum::value>(arg_list);
}

std::optional<sxm::NumValue> sxm::numfn::leval_toreal(Range<sxm::LEvalNodeNum *> arg_list) {
    return proj_toreal<sxm::LEvalNodeNum, &sxm::LEvalNodeNum::value>(arg_list);
}

namespace {

template <typename Arg, auto Member>
inline std::optional<sxm::NumValue> proj_bif(Range<Arg *> arg_list) {
    if (arg_list.size() == 0) {
        return std::nullopt;
    }

    for (Arg **arg_now = arg_list.p_start; arg_now < arg_list.p_end; ++arg_now) {
        if (std::abs((*arg_list.p_start->*Member).real) > sxm::NumValue::TOLERANCE_ABS) {
            return (*arg_list.p_start->*Member);
        }
    }

    return *arg_list.p_start->*Member;
}

}

std::optional<sxm::NumValue> sxm::numfn::bif(Range<sxm::NumValue> arg_list) {
    if (arg_list.size() == 0) {
        return std::nullopt;
    }

    for (; arg_list.p_start < arg_list.p_end - 1; ++arg_list.p_start) {
        if (std::abs(arg_list.p_start->real) > sxm::NumValue::TOLERANCE_ABS) {
            return *arg_list.p_start;
        }
    }

    return *arg_list.p_start;
}

std::optional<sxm::NumValue> sxm::numfn::feval_bif(Range<sxm::FEvalNodeNum *> arg_list) {
    return proj_bif<sxm::FEvalNodeNum, &sxm::FEvalNodeNum::value>(arg_list);
}

std::optional<sxm::NumValue> sxm::numfn::leval_bif(Range<sxm::LEvalNodeNum *> arg_list) {
    return proj_bif<sxm::LEvalNodeNum, &sxm::LEvalNodeNum::value>(arg_list);
}

namespace {

template <typename Arg, auto Member>
inline std::optional<sxm::NumValue> proj_bife(Range<Arg *> arg_list) {
    if (arg_list.size() != 3) {
        return std::nullopt;
    }

    if (std::abs((arg_list.p_start[0]->*Member).real) > sxm::NumValue::TOLERANCE_ABS) {
        return arg_list.p_start[1]->*Member;
    }

    return arg_list.p_start[2]->*Member;
}

}

std::optional<sxm::NumValue> sxm::numfn::bife(Range<sxm::NumValue> arg_list) {
    if (arg_list.size() != 3) {
        return std::nullopt;
    }

    if (std::abs(arg_list.p_start->real) > sxm::NumValue::TOLERANCE_ABS) {
        return arg_list.p_start[1];
    }

    return arg_list.p_start[2];
}

std::optional<sxm::NumValue> sxm::numfn::feval_bife(Range<sxm::FEvalNodeNum *> arg_list) {
    return proj_bife<sxm::FEvalNodeNum, &sxm::FEvalNodeNum::value>(arg_list);
}

std::optional<sxm::NumValue> sxm::numfn::leval_bife(Range<sxm::LEvalNodeNum *> arg_list) {
    return proj_bife<sxm::LEvalNodeNum, &sxm::LEvalNodeNum::value>(arg_list);
}

namespace {

template <typename Arg, auto Member>
inline std::optional<sxm::NumValue> proj_ctan(Range<Arg *> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue { .real = 1 / std::tan((*arg_list.p_start->*Member).real), };
}

}

std::optional<sxm::NumValue> sxm::numfn::ctan(Range<sxm::NumValue> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue { .real = 1 / std::tan(arg_list.p_start->real), };
}

std::optional<sxm::NumValue> sxm::numfn::feval_ctan(Range<sxm::FEvalNodeNum *> arg_list) {
    return proj_ctan<sxm::FEvalNodeNum, &sxm::FEvalNodeNum::value>(arg_list);
}

std::optional<sxm::NumValue> sxm::numfn::leval_ctan(Range<sxm::LEvalNodeNum *> arg_list) {
    return proj_ctan<sxm::LEvalNodeNum, &sxm::LEvalNodeNum::value>(arg_list);
}

namespace {

template <typename Arg, auto Member>
inline std::optional<sxm::NumValue> proj_tan(Range<Arg *> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue { .real = std::tan((*arg_list.p_start->*Member).real), };
}

}

std::optional<sxm::NumValue> sxm::numfn::tan(Range<sxm::NumValue> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue { .real = std::tan(arg_list.p_start->real), };
}

std::optional<sxm::NumValue> sxm::numfn::feval_tan(Range<sxm::FEvalNodeNum *> arg_list) {
    return proj_tan<sxm::FEvalNodeNum, &sxm::FEvalNodeNum::value>(arg_list);
}

std::optional<sxm::NumValue> sxm::numfn::leval_tan(Range<sxm::LEvalNodeNum *> arg_list) {
    return proj_tan<sxm::LEvalNodeNum, &sxm::LEvalNodeNum::value>(arg_list);
}

namespace {

template <typename Arg, auto Member>
inline std::optional<sxm::NumValue> proj_cos(Range<Arg *> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue { .real = std::cos((*arg_list.p_start->*Member).real), };
}

}

std::optional<sxm::NumValue> sxm::numfn::cos(Range<sxm::NumValue> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue { .real = std::cos(arg_list.p_start->real), };
}

std::optional<sxm::NumValue> sxm::numfn::feval_cos(Range<sxm::FEvalNodeNum *> arg_list) {
    return proj_cos<sxm::FEvalNodeNum, &sxm::FEvalNodeNum::value>(arg_list);
};

std::optional<sxm::NumValue> sxm::numfn::leval_cos(Range<sxm::LEvalNodeNum *> arg_list) {
    return proj_cos<sxm::LEvalNodeNum, &sxm::LEvalNodeNum::value>(arg_list);
};

namespace {

template <typename Arg, auto Member>
inline std::optional<sxm::NumValue> proj_sin(Range<Arg *> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue { .real = std::sin((*arg_list.p_start->*Member).real), };
}

}

std::optional<sxm::NumValue> sxm::numfn::sin(Range<sxm::NumValue> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue { .real = std::sin(arg_list.p_start->real), };
}

std::optional<sxm::NumValue> sxm::numfn::feval_sin(Range<sxm::FEvalNodeNum *> arg_list) {
    return proj_sin<sxm::FEvalNodeNum, &sxm::FEvalNodeNum::value>(arg_list);
}

std::optional<sxm::NumValue> sxm::numfn::leval_sin(Range<sxm::LEvalNodeNum *> arg_list) {
    return proj_sin<sxm::LEvalNodeNum, &sxm::LEvalNodeNum::value>(arg_list);
}

namespace {

template <typename Arg, auto Member>
inline std::optional<sxm::NumValue> proj_log(Range<Arg *> arg_list) {
    if (arg_list.size() != 2) {
        std::printf("BADSIZE: %ld\n", arg_list.size());
        return std::nullopt;
    }

    return sxm::NumValue {
        .real = std::log((arg_list.p_start[1]->*Member).real) / std::log((*arg_list.p_start->*Member).real),
    };
}

}

std::optional<sxm::NumValue> sxm::numfn::log(Range<sxm::NumValue> arg_list) {
    if (arg_list.size() != 2) {
        return std::nullopt;
    }

    return sxm::NumValue {
        .real = std::log((arg_list.p_start + 1)->real) / std::log(arg_list.p_start->real),
    };
}

std::optional<sxm::NumValue> sxm::numfn::feval_log(Range<sxm::FEvalNodeNum *> arg_list) {
    return proj_log<sxm::FEvalNodeNum, &sxm::FEvalNodeNum::value>(arg_list);
}

std::optional<sxm::NumValue> sxm::numfn::leval_log(Range<sxm::LEvalNodeNum *> arg_list) {
    return proj_log<sxm::LEvalNodeNum, &sxm::LEvalNodeNum::value>(arg_list);
}

namespace {

template <typename Arg, auto Member>
inline std::optional<sxm::NumValue> proj_ln(Range<Arg *> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue {
        .real = std::log((*arg_list.p_start->*Member).real),
    };
}

}

std::optional<sxm::NumValue> sxm::numfn::ln(Range<sxm::NumValue> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue {
        .real = std::log(arg_list.p_start->real),
    };
}

std::optional<sxm::NumValue> sxm::numfn::feval_ln(Range<sxm::FEvalNodeNum *> arg_list) {
    return proj_ln<sxm::FEvalNodeNum, &sxm::FEvalNodeNum::value>(arg_list);
}

std::optional<sxm::NumValue> sxm::numfn::leval_ln(Range<sxm::LEvalNodeNum *> arg_list) {
    return proj_ln<sxm::LEvalNodeNum, &sxm::LEvalNodeNum::value>(arg_list);
}

namespace {

template <typename Arg, auto Member>
inline std::optional<sxm::NumValue> proj_log10(Range<Arg *> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue {
        .real = std::log10((*arg_list.p_start->*Member).real),
    };
}

}

std::optional<sxm::NumValue> sxm::numfn::log10(Range<sxm::NumValue> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue {
        .real = std::log10(arg_list.p_start->real),
    };
}

std::optional<sxm::NumValue> sxm::numfn::feval_log10(Range<sxm::FEvalNodeNum *> arg_list) {
    return proj_log10<sxm::FEvalNodeNum, &sxm::FEvalNodeNum::value>(arg_list);
}

std::optional<sxm::NumValue> sxm::numfn::leval_log10(Range<sxm::LEvalNodeNum *> arg_list) {
    return proj_log10<sxm::LEvalNodeNum, &sxm::LEvalNodeNum::value>(arg_list);
}

namespace {

template <typename Arg, auto Member>
inline std::optional<sxm::NumValue> proj_log2(Range<Arg *> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue {
        .real = std::log2((*arg_list.p_start->*Member).real),
    };
}

}

std::optional<sxm::NumValue> sxm::numfn::log2(Range<sxm::NumValue> arg_list) {
    if (arg_list.size() != 1) {
        return std::nullopt;
    }

    return sxm::NumValue {
        .real = std::log2(arg_list.p_start->real),
    };
}

std::optional<sxm::NumValue> sxm::numfn::feval_log2(Range<sxm::FEvalNodeNum *> arg_list) {
    return proj_log2<sxm::FEvalNodeNum, &sxm::FEvalNodeNum::value>(arg_list);
}

std::optional<sxm::NumValue> sxm::numfn::leval_log2(Range<sxm::LEvalNodeNum *> arg_list) {
    return proj_log2<sxm::LEvalNodeNum, &sxm::LEvalNodeNum::value>(arg_list);
}

std::optional<sxm::NumValue> sxm::numfn::b(Range<sxm::NumValue>) {
    return NumValue { 0, };
}

std::optional<sxm::NumValue> sxm::numfn::feval_b(Range<sxm::FEvalNodeNum *>) {
    return NumValue { 0, };
}

std::optional<sxm::NumValue> sxm::numfn::leval_b(Range<sxm::LEvalNodeNum *>) {
    return NumValue { 0, };
}
