#pragma once
#include <cstdint>
#include <cstddef>

//
// FPGA math utilities implementing VHDL functions 1:1
// using only unsigned __int128 for all computations.
//
// All functions are constexpr and safe at compile time.
//

struct FpgaMath {

    // ----------------------------------------------------
    // max2bits(a)
    // Return number of bits needed to represent unsigned value a.
    // ----------------------------------------------------
    static consteval size_t max2bits(unsigned __int128 a) {
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
    static consteval unsigned __int128 fit(unsigned __int128 a,
                                           size_t /*extra_bits*/ = 0)
    {
        return a; // resizing does nothing in C++
    }

    // ----------------------------------------------------
    // sum_x(x1, x2)
    // General VHDL version: sum of integers from x1..x2
    // ----------------------------------------------------
    static consteval unsigned __int128 sum_x(unsigned __int128 x1,
                                             unsigned __int128 x2)
    {
        return ( (x2 - x1 + 1) * (x1 + x2) ) / 2;
    }

    // ----------------------------------------------------
    // sum_x2(x1, x2)
    // Sum of squares from x1..x2
    // ----------------------------------------------------
    static consteval unsigned __int128 sum_x2(unsigned __int128 x1,
                                              unsigned __int128 x2)
    {
        // VHDL formula:
        // x2*(x2+1)*(2*x2+1)/6 - (x1-1)*x1*(2*x1-1)/6
        unsigned __int128 A = x2;
        unsigned __int128 B = x1;

        unsigned __int128 termA =
            A * (A + 1) * (2 * A + 1) / 6;

        unsigned __int128 termB =
            (B - 1) * B * (2 * B - 1) / 6;

        return termA - termB;
    }

    // ----------------------------------------------------
    // sum_xy
    // Sum over X range × sum over Y range
    // ----------------------------------------------------
    static consteval unsigned __int128 sum_xy(unsigned __int128 x1,
                                              unsigned __int128 x2,
                                              unsigned __int128 y1,
                                              unsigned __int128 y2)
    {
        return sum_x(x1, x2) * sum_x(y1, y2);
    }
};

