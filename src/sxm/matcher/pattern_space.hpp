#include "sxm/matcher/pattern.hpp"

#pragma once

namespace sxm {

template <Pattern_Kind idx_next>

std::optional<Pattern_Match> pattern_space(const char *src, const Pattern_Context *) {
    const char *src_now = src;

    if (*src_now == ' ') {
        while (*(++src_now) == ' ') {};

        return Pattern_Match{
            .token = {
                .kind = Token_Kind::Space,
                .section_start = src,
                .section_end = src_now,
            },

            .idx_next = idx_next,
        };
    }

    return std::nullopt;
};

}
