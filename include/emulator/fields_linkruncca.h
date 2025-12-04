#pragma once

#include <util/FpgaMath.h>

#include "fields.h"

struct FpgaGenerics_linkruncca {
    size_t X_SIZE;
    size_t Y_BITS;
};

template <FpgaGenerics_linkruncca generics>
class fields_linkruncca {
public:
    struct FpgaConstants {
        static constexpr size_t X_SIZE = generics.X_SIZE;
        static constexpr size_t Y_BITS = generics.Y_BITS;
        
        static constexpr size_t X_BITS = FpgaMath::max2bits(X_SIZE - 1);
        static constexpr size_t Y_LOW_BITS = Y_BITS - 1;
        static constexpr size_t Y_SIZE = ((size_t)1) << Y_BITS;
        static constexpr size_t Y_MAX = Y_SIZE - 1;
        static constexpr size_t Y_LOW_SIZE = ((size_t)1) << Y_LOW_BITS;
        static constexpr size_t Y_LOW_MAX = Y_LOW_SIZE - 1;
        static constexpr size_t N_SEG_SUM_BITS = FpgaMath::max2bits(FpgaMath::fit(X_SIZE)*FpgaMath::fit(Y_LOW_SIZE));

        static constexpr size_t MEM_ADD_BITS = X_BITS - 1;

        static constexpr size_t X2_SUM_BITS = FpgaMath::max2bits(FpgaMath::sum_x2(0, X_SIZE - 1) * FpgaMath::fit(Y_SIZE));
        static constexpr size_t YLOW2_SUM_BITS = FpgaMath::max2bits(FpgaMath::sum_x2(0, Y_LOW_MAX) * FpgaMath::fit(X_SIZE));
        static constexpr size_t XYLOW_SUM_BITS = FpgaMath::max2bits(FpgaMath::sum_xy(0, X_SIZE - 1, 0, Y_LOW_MAX));
        static constexpr size_t X_SEG_SUM_BITS = FpgaMath::max2bits(FpgaMath::sum_x(0, X_SIZE - 1) * FpgaMath::fit(Y_LOW_SIZE));
        static constexpr size_t YLOW_SEG_SUM_BITS = FpgaMath::max2bits(FpgaMath::sum_x(0, Y_LOW_MAX) * X_SIZE);
    };

enum class wr_fields: size_t {
        RST,
        DATAVALID,
        IN_LABEL,
        X,
        Y,
        HAS_RED,
        HAS_GREEN,
        HAS_BLUE,
        END_OF_FIELDS // The last element must be END_OF_FIELDS, which is not a real field
    };
    
    enum class rd_fields: size_t {
        VALID,
        X_LEFT,
        X_RIGHT,
        Y_TOP_SEG_0,
        Y_TOP_SEG_1,
        Y_BOTTOM_SEG_0,
        Y_BOTTOM_SEG_1,
        X2_SUM,
        YLOW2_SUM,
        XYLOW_SUM,
        X_SEG0_SUM,
        X_SEG1_SUM,
        YLOW_SEG0_SUM,
        YLOW_SEG1_SUM,
        N_SEG0_SUM,
        N_SEG1_SUM,
        END_OF_FIELDS // The last element must be END_OF_FIELDS, which is not a real field
    };

    consteval static
    auto get_wr_specs()
    {
        return std::to_array<FieldSpec<wr_fields>>({
            { wr_fields::RST, 1 },
            { wr_fields::DATAVALID, 1 },
            { wr_fields::IN_LABEL, 1 },
            { wr_fields::X, FpgaConstants::X_BITS },
            { wr_fields::Y, FpgaConstants::Y_BITS },
            { wr_fields::HAS_RED, 1 },
            { wr_fields::HAS_GREEN, 1 },
            { wr_fields::HAS_BLUE, 1 },
        });
    }

    consteval auto static
    get_rd_specs() {
        return std::to_array<FieldSpec<rd_fields>>({
            { rd_fields::VALID, 1 },
            { rd_fields::X_LEFT, FpgaConstants::X_BITS },
            { rd_fields::X_RIGHT, FpgaConstants::X_BITS },
            { rd_fields::Y_TOP_SEG_0, FpgaConstants::Y_LOW_BITS },
            { rd_fields::Y_TOP_SEG_1, FpgaConstants::Y_LOW_BITS },
            { rd_fields::Y_BOTTOM_SEG_0, FpgaConstants::Y_LOW_BITS },
            { rd_fields::Y_BOTTOM_SEG_1, FpgaConstants::Y_LOW_BITS },
            { rd_fields::X2_SUM, FpgaConstants::X2_SUM_BITS },
            { rd_fields::YLOW2_SUM, FpgaConstants::YLOW2_SUM_BITS },
            { rd_fields::XYLOW_SUM, FpgaConstants::XYLOW_SUM_BITS },
            { rd_fields::X_SEG0_SUM, FpgaConstants::X_SEG_SUM_BITS },
            { rd_fields::X_SEG1_SUM, FpgaConstants::X_SEG_SUM_BITS },
            { rd_fields::YLOW_SEG0_SUM, FpgaConstants::YLOW_SEG_SUM_BITS },
            { rd_fields::YLOW_SEG1_SUM, FpgaConstants::YLOW_SEG_SUM_BITS },
            { rd_fields::N_SEG0_SUM, FpgaConstants::N_SEG_SUM_BITS },
            { rd_fields::N_SEG1_SUM, FpgaConstants::N_SEG_SUM_BITS },
        });
    };
};
