#pragma once

#include "fields.h"

template<typename HW, typename FIELDS>
class emulator_fields {
    using shadow_t = shadow<HW, FIELDS>;
public:
    using wr_raw_t = typename shadow_t::wr_word_t;
    using rd_raw_t = typename shadow_t::rd_word_t;

    using wr_fields = typename FIELDS::wr_fields;
    using rd_fields = typename FIELDS::rd_fields;

    using fields_t = fields<FIELDS>;

    emulator_fields(HW &hw) : 
        hw_(hw), shadow_(hw), slicer_(shadow_)
    {}

    template<typename word_t>
    inline void wr_field(fields_t::wr_fields field, word_t data) {
        auto desc = fields_t::wr_desc(field);
        slicer_.write_bits(desc.bit_offset, desc.bit_width, data);
    }

    template<typename word_t>
    inline void rd_field(fields_t::rd_fields field, word_t &data) {
        auto desc = fields_t::rd_desc(field);
        data = slicer_.template read_bits<word_t>(desc.bit_offset, desc.bit_width);
    }

    inline void wr_flush() {
        slicer_.wr_flush();
    }

    inline void rd_flush() {
        slicer_.rd_flush();
    }

    inline void wr_raw(size_t word_address, wr_raw_t data) {
        shadow_.wr_raw(word_address, data);
    }

    inline rd_raw_t rd_raw(size_t word_address) {
        return shadow_.rd_raw(word_address);
    }
private:

    HW &hw_;
    shadow_t shadow_;
    bit_slicer<shadow_t> slicer_;
};
