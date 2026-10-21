// #include "algo/list/range.hpp"
// #include <algorithm>
//
// #pragma once
//
// template <typename val_t>
//
// struct SArray {
//     Range<val_t> data;
//
//     SArray() noexcept
//         : data {} {
//     }
//
//     SArray(std::size_t size) noexcept {
//         this->data.p_start = new val_t[size];
//         this->data.p_end = this->data.p_start + size;
//     }
//
//     // copy
//     SArray(const SArray<val_t> &src) noexcept {
//         val_t *cpy_now = new val_t[src.data.size()];
//
//         this->data.p_start = cpy_now;
//         this->data.p_end = cpy_now + src.data.size();
//
//         try {
//             std::copy(src.data.p_start, src.data.p_end, this->data.p_start);
//         } catch (...) {
//             delete[] cpy_now;
//
//             this->data.p_start = nullptr;
//             this->data.p_end = nullptr;
//         }
//     }
//
//     SArray& operator=(const SArray<val_t> &src) noexcept {
//         val_t *cpy_now = new val_t[src.data.size()];
//
//         this->data.p_start = cpy_now;
//         this->data.p_end = cpy_now + src.data.size();
//
//         try {
//             std::copy(src.data.p_start, src.data.p_end, this->data.p_start);
//         } catch (...) {
//             delete[] cpy_now;
//
//             this->data.p_start = nullptr;
//             this->data.p_end = nullptr;
//         }
//
//         return *this;
//     }
//
//     // move
//     SArray(SArray<val_t> &&src) noexcept {
//         this->data.p_start = src.data.p_start;
//         this->data.p_end = src.data.p_end;
//
//         src.data.p_start = nullptr;
//         src.data.p_end = nullptr;
//
//         return *this;
//     }
//
//     SArray& operator=(SArray<val_t> &&src) noexcept {
//         this->data.p_start = src.data.p_start;
//         this->data.p_end = src.data.p_end;
//
//         src.data.p_start = nullptr;
//         src.data.p_end = nullptr;
//
//         return *this;
//     }
//
//     ~SArray() {
//         delete[] this->data.p_start;
//     }
// };
