#pragma once

namespace sxm {

class NumValue {
  public:
    static constexpr double TOLERANCE_ABS = 1e-12;
    static constexpr double TOLERANCE_ABS_NEG = -1e-12;

    double real;

    static NumValue parse_int(const char *name_start, const char *name_end);
    static NumValue parse_real(const char *name_start, const char *name_end);

    static NumValue neg(const NumValue *child);
    static NumValue add(const NumValue *left, const NumValue *right);
    static NumValue sub(const NumValue *left, const NumValue *right);
    static NumValue mul(const NumValue *left, const NumValue *right);
    static NumValue div(const NumValue *left, const NumValue *right);
    static NumValue mod(const NumValue *left, const NumValue *right);
    static NumValue pow(const NumValue *left, const NumValue *right);

    static NumValue gt(const NumValue *left, const NumValue *right);
    static NumValue gte(const NumValue *left, const NumValue *right);
    static NumValue lt(const NumValue *left, const NumValue *right);
    static NumValue lte(const NumValue *left, const NumValue *right);

    static NumValue eq(const NumValue *left, const NumValue *right);
    static NumValue ueq(const NumValue *left, const NumValue *right);
};

}
