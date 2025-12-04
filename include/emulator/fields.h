#pragma once

template <typename T>
struct FieldSpec {
    T field;
    size_t bit_width;
};

struct FieldDesc {
    size_t bit_offset;
    size_t bit_width;
};

template<typename fields_def>
class fields {
public:
    using wr_fields = typename fields_def::wr_fields;
    using rd_fields = typename fields_def::rd_fields;

    template <typename T>
    consteval static auto spec_bits(T specs) {
        size_t bits = 0;
        for(const auto &spec: specs) {
            bits += spec.bit_width;
        }
        return bits;
    }

    static constexpr auto wr_specs = fields_def::get_wr_specs();
    static constexpr auto rd_specs = fields_def::get_rd_specs();

    static constexpr auto wr_bits = spec_bits(wr_specs);
    static constexpr auto rd_bits = spec_bits(rd_specs);

    template <typename T>
    consteval static auto get_field_descs(T specs) {
        size_t bit_offset = 0;
        constexpr auto len = specs.size();
        std::array<FieldDesc, len> descs;

        for (size_t i=0; i<len; i++) {
            auto bit_width = specs[i].bit_width;
            descs[i].bit_offset = bit_offset;
            descs[i].bit_width = bit_width;
            bit_offset += bit_width;
        }

        return descs;
    }

    static constexpr auto wr_descs = get_field_descs(wr_specs);
    static constexpr auto rd_descs = get_field_descs(rd_specs);

    static constexpr size_t num_wr_fields = 
        static_cast<size_t>(wr_fields::END_OF_FIELDS);

    static constexpr size_t num_rd_fields = 
        static_cast<size_t>(rd_fields::END_OF_FIELDS);

    static_assert(wr_specs.size() == num_wr_fields, "Number of WR fields doesn't match.");
    static_assert(rd_specs.size() == num_rd_fields, "Number of WR fields doesn't match.");

    static constexpr auto wr_desc(wr_fields f) {
        return wr_descs[static_cast<size_t>(f)];
    }
    
    static constexpr auto rd_desc(rd_fields f) {
        return rd_descs[static_cast<size_t>(f)];
    }
};
