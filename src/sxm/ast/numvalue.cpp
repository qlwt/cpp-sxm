#include "sxm/ast/numvalue.hpp"
#include <charconv>
#include <cstdlib>
#include <cmath>

namespace {

constexpr sxm::NumValue PRIMITIVE_ONE = sxm::NumValue(1.0);
constexpr sxm::NumValue PRIMITIVE_ZERO = sxm::NumValue(0.0);

}

sxm::NumValue sxm::NumValue::parse_int(const char *src_now, const char* src_end) {
    double real;

    std::from_chars(src_now, src_end, real);

    return sxm::NumValue { real, };
}

sxm::NumValue sxm::NumValue::parse_real(const char *src_now, const char* src_end) {
    double real;

    std::from_chars(src_now, src_end, real);

    return sxm::NumValue { real, };
}

sxm::NumValue sxm::NumValue::add(const sxm::NumValue *left, const sxm::NumValue *right) {
    return sxm::NumValue{ left->real + right->real, };
}

sxm::NumValue sxm::NumValue::sub(const sxm::NumValue *left, const sxm::NumValue *right) {
    return sxm::NumValue{ left->real - right->real, };
}

sxm::NumValue sxm::NumValue::mul(const sxm::NumValue *left, const sxm::NumValue *right) {
    return sxm::NumValue { left->real * right->real, };
}

sxm::NumValue sxm::NumValue::mod(const sxm::NumValue *left, const sxm::NumValue *right) {
    return sxm::NumValue { std::fmod(left->real, right->real), };
}

sxm::NumValue sxm::NumValue::pow(const sxm::NumValue *left, const sxm::NumValue *right) {
    return sxm::NumValue { std::pow(left->real, right->real), };
}

sxm::NumValue sxm::NumValue::div(const sxm::NumValue *left, const sxm::NumValue *right) {
    return sxm::NumValue { left->real / right->real, };
}

sxm::NumValue sxm::NumValue::eq(const sxm::NumValue *left, const sxm::NumValue *right) {
    bool comp = std::abs(left->real - right->real) <= sxm::NumValue::TOLERANCE_ABS;

    if (comp) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}

sxm::NumValue sxm::NumValue::ueq(const sxm::NumValue *left, const sxm::NumValue *right) {
    bool comp = std::abs(left->real - right->real) > sxm::NumValue::TOLERANCE_ABS;

    if (comp) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}

sxm::NumValue sxm::NumValue::lt(const sxm::NumValue *left, const sxm::NumValue *right) {
    bool comp = (right->real - left->real) > sxm::NumValue::TOLERANCE_ABS;

    if (comp) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}

sxm::NumValue sxm::NumValue::lte(const sxm::NumValue *left, const sxm::NumValue *right) {
    bool comp = (right->real - left->real) >= sxm::NumValue::TOLERANCE_ABS_NEG;

    if (comp) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}

sxm::NumValue sxm::NumValue::gt(const sxm::NumValue *left, const sxm::NumValue *right) {
    bool comp = (left->real - right->real) > sxm::NumValue::TOLERANCE_ABS;

    if (comp) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}

sxm::NumValue sxm::NumValue::gte(const sxm::NumValue *left, const sxm::NumValue *right) {
    bool comp = (left->real - right->real) >= sxm::NumValue::TOLERANCE_ABS_NEG;

    if (comp) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}

sxm::NumValue sxm::NumValue::neg(const sxm::NumValue *child) {
    return NumValue { -child->real, };
}
