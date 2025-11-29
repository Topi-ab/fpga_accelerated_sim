#pragma once

#include <stdexcept>
#include <format> 

#include "fields.h"

template<typename hw_access_t, typename fields_t>
class shadow {
public:
    using wr_word_t = typename hw_access_t::wr_word_t;
    using rd_word_t = typename hw_access_t::rd_word_t;

    constexpr shadow(hw_access_t &hw) : hw_(hw) {
        wr_cache_.fill(0);
        wr_dirty_.fill(true);
        rd_dirty_.fill(true);
    }

    void inline write(size_t word_offset, wr_word_t data, wr_word_t mask) {
        if(word_offset >= wr_entries) {
            throw std::runtime_error(std::format("shadow::write() word_offset ({}) out of range", word_offset));
        }
        auto &cache = wr_cache_[word_offset];
        cache = (cache & ~mask) | (data & mask);
        wr_dirty_[word_offset] = true;
    }

    void inline wr_flush() {
        for(size_t idx = 0; idx < wr_entries; idx++) {
            if(wr_dirty_[idx]) {
                hw_.wr(idx, wr_cache_[idx]);
                wr_dirty_[idx] = false;
            }
        }
    }

    void inline wr_raw(size_t word_address, wr_word_t data) {
      hw_.wr_raw(word_address, data);
    }

    rd_word_t inline read(size_t word_offset) {
        if(word_offset >= rd_entries) {
            throw std::runtime_error(std::format("shadow::read() word_offset ({}) out of range", word_offset));
        }
        auto idx = word_offset;
        if(rd_dirty_[idx]) {
            rd_cache_[idx] = hw_.rd(idx);
            rd_dirty_[idx] = false;
        }
        return rd_cache_[idx];
    }

    void inline rd_flush() noexcept {
        rd_dirty_.fill(true);
    }

    rd_word_t rd_raw(size_t word_address) {
        return hw_.rd_raw(word_address);
    }
private:
    hw_access_t &hw_;

    static constexpr size_t wr_bits = fields<fields_t>::wr_bits;
    static constexpr size_t rd_bits = fields<fields_t>::rd_bits;

    static constexpr size_t WR_BITS_PER_WORD = sizeof(wr_word_t) * 8;
    static constexpr size_t RD_BITS_PER_WORD = sizeof(rd_word_t) * 8;
    
    static constexpr size_t wr_entries = (wr_bits + WR_BITS_PER_WORD - 1) / WR_BITS_PER_WORD;
    static constexpr size_t rd_entries = (rd_bits + RD_BITS_PER_WORD - 1) / RD_BITS_PER_WORD;

    std::array<wr_word_t, wr_entries> wr_cache_;
    std::array<rd_word_t, rd_entries> rd_cache_;
    std::array<bool, wr_entries> wr_dirty_;
    std::array<bool, rd_entries> rd_dirty_;
};

