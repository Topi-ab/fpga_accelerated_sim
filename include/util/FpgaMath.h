#pragma once
#include <cstdint>
#include <cstddef>

// ------------------------------------------------------------
// HELPER FUNCTIONS FOR CALCULATING REQUIRED BIT WIDTHS
// ------------------------------------------------------------

//
// FPGA math utilities implementing VHDL functions 1:1
// using only unsigned __int128 for all computations.
//
// All functions are constexpr and safe at compile time.
//

struct FpgaMath {
    using local_uint_t = unsigned __int128;

    // ----------------------------------------------------
    // max2bits(a)
    // Return number of bits needed to represent unsigned value a.
    // ----------------------------------------------------
    static consteval size_t max2bits(local_uint_t a) {
        if (a == 0) return 0;

        size_t bits = 0;
        while (a > 0) {
            a >>= 1;
            bits++;
        }
        return bits;
    }

    // ----------------------------------------------------
    // fit(a, extra_bits)
    //
    // VHDL uses "fit" to resize vectors. For C++, we only need
    // the semantic side effect on bit-width algebra.
    //
    // Returning the same value is fine.
    // ----------------------------------------------------
    static consteval local_uint_t fit(local_uint_t a,
        size_t /*extra_bits*/ = 0)
    {
        return a; // resizing does nothing in C++
    }

    // ----------------------------------------------------
    // sum_x(x1, x2)
    // General VHDL version: sum of integers from x1..x2
    // ----------------------------------------------------
    static consteval local_uint_t sum_x(local_uint_t x1,
        local_uint_t x2)
    {
        return ( (x2 - x1 + 1) * (x1 + x2) ) / 2;
    }

    // ----------------------------------------------------
    // sum_x2(x1, x2)
    // Sum of squares from x1..x2
    // ----------------------------------------------------
    static consteval local_uint_t sum_x2(local_uint_t x1,
        local_uint_t x2)
    {
        // VHDL formula:
        // x2*(x2+1)*(2*x2+1)/6 - (x1-1)*x1*(2*x1-1)/6
        local_uint_t A = x2;
        local_uint_t B = x1;

        local_uint_t termA =
            A * (A + 1) * (2 * A + 1) / 6;

        local_uint_t termB =
            (B - 1) * B * (2 * B - 1) / 6;

        return termA - termB;
    }

    // ----------------------------------------------------
    // sum_xy
    // Sum over X range × sum over Y range
    // ----------------------------------------------------
    static consteval local_uint_t sum_xy(local_uint_t x1,
        local_uint_t x2,
        local_uint_t y1,
        local_uint_t y2)
{
        return sum_x(x1, x2) * sum_x(y1, y2);
    }
};

