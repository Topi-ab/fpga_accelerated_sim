#pragma once

#include <PacketStructAccessor/PacketStructAccessor.h>

#include "LinkRunCCA_IF.h"

template <typename Backend, typename IF>
class FpgaInterface {
    constexpr static auto wr_desc = IF::get_wr_desc();
    constexpr static auto rd_desc = IF::get_rd_desc();
public:

    template<typename word_t>
    void write(IF::LinkRunCCA_collect_t_fields field, word_t value) {
        const auto& desc = wr_desc[(size_t)field];
        if(value >> desc.width) {
            throw std::runtime_error("FpgaInterface::write: value too large for field");
        }
        acc_.write_bits(desc.offset, desc.width, value);
    }

    void wr_flush() {
        acc_.wr_flush();
    }

    template<typename word_t>
    void read(IF::LinkRunCCA_feature_t_fields field, word_t& value) {
        const auto& desc = rd_desc[(size_t)field];
        acc_.read_bits(desc.offset, desc.width, value);
    }

    void rd_flush() {
        acc_.rd_flush();
    }

    FpgaInterface(Backend& backend)
        : backend_(backend),
          acc_(backend, IF::wr_bits(), IF::rd_bits())
    {}

private:
    Backend& backend_;
    PackedStructAccessor<Backend> acc_;
};
