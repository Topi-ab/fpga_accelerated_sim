#pragma once
#include <cstddef>


struct FpgaGenerics {
    const size_t X_SIZE;
    const size_t Y_BITS;

    constexpr FpgaGenerics(size_t x_size, size_t y_bits)
        : X_SIZE(x_size), Y_BITS(y_bits)
    {}
};

