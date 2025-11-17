#pragma once
#include <cstddef>
#include <array>
#include <cassert>

#include "util/FpgaMath.h"

#include "FpgaGenerics.h"

template <FpgaGenerics generics>
class LinkRunCCA_IF {
    template <typename T>
    struct FieldSpec {
        T field;
        size_t width;
    };

    struct FieldDesc {
        size_t offset;
        size_t width;
    };

public:

// ------------------------------------------------------------
//  APPLICATION-SPECIFIC DEFINITIONS OF INPUT AND OUTPUT FIELDS
//  START HERE
// ------------------------------------------------------------

// Input fields (to FPGA)
    enum class LinkRunCCA_collect_t_fields : size_t {
        IN_LABEL,
        X,
        Y,
        HAS_RED,
        HAS_GREEN,
        HAS_BLUE,
        END_OF_FIELDS // Last element must be END_OF_FIELDS, which is not a real field
    };

// Output fields (from FPGA)
    enum class LinkRunCCA_feature_t_fields : size_t {
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
        END_OF_FIELDS // Last element must be END_OF_FIELDS, which is not a real field
    };

    // THIS CODE RESEBLES CODE IN FPGA TO MATCH AUTO-CALCULATED FIELD SIZES
    struct FpgaConstants{
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
//------------------------------------------------------------
// APPLICATION-SPECIFIC DEFINITIONS OF INPUT AND OUTPUT FIELDS
// END HERE
//------------------------------------------------------------

    template <typename T>
    consteval static auto get_field_desc(T specs) {
        size_t offset = 0;
        std::array<FieldDesc, specs.size()> descs{};

        for (size_t i = 0; i < specs.size(); ++i) {
            descs[i].offset = offset;
            descs[i].width = specs[i].width;
            offset += specs[i].width;
        }

        return descs;
    }

    constexpr static FpgaConstants consts{};

    static constexpr size_t NUM_FIELDS =
    static_cast<size_t>(LinkRunCCA_collect_t_fields::END_OF_FIELDS);
    
    typedef std::array<FieldSpec<LinkRunCCA_collect_t_fields>, NUM_FIELDS> 
    LinkRunCCA_collect_t_field_specs_t;
    
    typedef std::array<FieldSpec<LinkRunCCA_feature_t_fields>, (size_t)LinkRunCCA_feature_t_fields::END_OF_FIELDS> 
    LinkRunCCA_feature_t_field_specs_t;
    
    consteval auto static
    get_LinkrunCCA_collect_specs()
    {
        using fields = LinkRunCCA_collect_t_fields;
//------------------------------------------------------------
// APPLICATION-SPECIFIC FIELD SPECS START HERE
//------------------------------------------------------------
        auto constexpr r = std::to_array<FieldSpec<fields>>({
            { fields::IN_LABEL, 1 },
            { fields::X, consts.X_BITS },
            { fields::Y, consts.Y_BITS },
            { fields::HAS_RED, 1 },
            { fields::HAS_GREEN, 1 },
            { fields::HAS_BLUE, 1 },
        });
//------------------------------------------------------------
// APPLICATION-SPECIFIC FIELD SPECS END HERE
//------------------------------------------------------------

        static_assert([=]{
            for (std::size_t i = 0; i < r.size(); ++i)
                if ((std::size_t)r[i].field != i)
                    return false;
            return true;
        }(), "Collect field definitions out of order");

        static_assert(r.size() == (size_t)fields::END_OF_FIELDS, "Collect fields: wrong number of fields");

        return r;
    }

    consteval auto static
    get_LinkrunCCA_feature_specs()
    {
        using fields = LinkRunCCA_feature_t_fields;

//------------------------------------------------------------
// APPLICATION-SPECIFIC FIELD SPECS START HERE
//------------------------------------------------------------
        auto constexpr r = std::to_array<FieldSpec<fields>>({
            { LinkRunCCA_feature_t_fields::X_LEFT, consts.X_BITS },
            { LinkRunCCA_feature_t_fields::X_RIGHT, consts.X_BITS },
            { LinkRunCCA_feature_t_fields::Y_TOP_SEG_0, consts.Y_LOW_BITS },
            { LinkRunCCA_feature_t_fields::Y_TOP_SEG_1, consts.Y_LOW_BITS },
            { LinkRunCCA_feature_t_fields::Y_BOTTOM_SEG_0, consts.Y_LOW_BITS },
            { LinkRunCCA_feature_t_fields::Y_BOTTOM_SEG_1, consts.Y_LOW_BITS },
            { LinkRunCCA_feature_t_fields::X2_SUM, consts.X2_SUM_BITS },
            { LinkRunCCA_feature_t_fields::YLOW2_SUM, consts.YLOW2_SUM_BITS },
            { LinkRunCCA_feature_t_fields::XYLOW_SUM, consts.XYLOW_SUM_BITS },
            { LinkRunCCA_feature_t_fields::X_SEG0_SUM, consts.X_SEG_SUM_BITS },
            { LinkRunCCA_feature_t_fields::X_SEG1_SUM, consts.X_SEG_SUM_BITS },
            { LinkRunCCA_feature_t_fields::YLOW_SEG0_SUM, consts.YLOW_SEG_SUM_BITS },
            { LinkRunCCA_feature_t_fields::YLOW_SEG1_SUM, consts.YLOW_SEG_SUM_BITS },
            { LinkRunCCA_feature_t_fields::N_SEG0_SUM, consts.N_SEG_SUM_BITS },
            { LinkRunCCA_feature_t_fields::N_SEG1_SUM, consts.N_SEG_SUM_BITS },
        });
//------------------------------------------------------------
// APPLICATION-SPECIFIC FIELD SPECS END HERE
//------------------------------------------------------------

        static_assert([=]{
            for (std::size_t i = 0; i < r.size(); ++i)
                if ((std::size_t)r[i].field != i)
                    return false;
            return true;
        }(), "Feature field definitions out of order");

        static_assert(r.size() == (size_t)fields::END_OF_FIELDS, "Feature fields: wrong number of fields");

        return r;
    }

public:
    consteval static auto get_wr_desc() {
        constexpr auto specs = get_LinkrunCCA_collect_specs();
        return get_field_desc(specs);
    }

    consteval static auto get_rd_desc() {
        constexpr auto specs = get_LinkrunCCA_feature_specs();
        return get_field_desc(specs);
    }

    consteval static auto wr_bits() {
        constexpr auto specs = get_LinkrunCCA_collect_specs();
        size_t total = 0;
        for (const auto& spec : specs) {
            total += spec.width;
        }
        return total;
    }

    consteval static auto rd_bits() {
        constexpr auto specs = get_LinkrunCCA_feature_specs();
        size_t total = 0;
        for (const auto& spec : specs) {
            total += spec.width;
        }
        return total;
    }
};
