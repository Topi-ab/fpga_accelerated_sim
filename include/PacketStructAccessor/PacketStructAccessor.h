#pragma once
#include <cstddef>
#include <cstdint>
#include <stdexcept>

template <typename Backend>
class PackedStructAccessor {
public:
    PackedStructAccessor(Backend& backend,
                         size_t wr_bits,
                         size_t rd_bits)
        : backend_(backend)
    {
        backend_.init(wr_bits, rd_bits);
    }

    // ------------------------------------------------------------
    // WRITE FIELD, AUTO-DETECTING WORD SIZE
    // ------------------------------------------------------------
    template <typename word_t>
    void write_bits(size_t offset, size_t width, word_t value)
    {
        using atomic_t = typename Backend::write_word_t;
        //static constexpr size_t ATOMIC_BITS = sizeof(word_t) * 8;
        static constexpr size_t ATOMIC_BITS = sizeof(atomic_t) * 8;

        auto start_index = offset / ATOMIC_BITS;
        auto end_index = (offset + width - 1) / ATOMIC_BITS;

        for (auto index = start_index; index <= end_index; ++index) {
            size_t bit_start = (index == start_index) ? (offset % ATOMIC_BITS) : 0;
            size_t bit_end = (index == end_index) ? ((offset + width - 1) % ATOMIC_BITS) : (ATOMIC_BITS - 1);
            size_t bit_width = bit_end - bit_start + 1;

            word_t mask = (bit_width == ATOMIC_BITS) ? ~word_t(0) : ((word_t(1) << bit_width) - word_t(1));
            word_t v = (value >> (bit_start + (index - start_index) * ATOMIC_BITS)) & mask;

            word_t wr_mask = mask << bit_start;
            word_t wr_data = v << bit_start;

            backend_.wr_write(index, wr_data, wr_mask);
        }
    }

    // ------------------------------------------------------------
    // READ FIELD, AUTO-DETECTING WORD SIZE
    // ------------------------------------------------------------
    template <typename word_t>
    void read_bits(size_t offset, size_t width, word_t& value)
    {
        using atomic_t = typename Backend::read_word_t;
        static constexpr size_t ATOMIC_BITS = sizeof(atomic_t) * 8;

        value = 0;

        if(width == 0)
            throw std::runtime_error("read_bits: wdith of zero not allowed");
        
        if(width > sizeof(word_t) * 8)
            throw std::runtime_error("read_bits: reading field wider than word_t read result type width");

        auto start_index = offset / ATOMIC_BITS;
        auto end_index = (offset + width - 1) / ATOMIC_BITS;

        for (auto index = start_index; index <= end_index; ++index) {
            atomic_t word;
            word = backend_.rd_read(index);
            size_t bit_start = (index == start_index) ? (offset % ATOMIC_BITS) : 0;
            size_t bit_end = (index == end_index) ? ((offset + width - 1) % ATOMIC_BITS) : (ATOMIC_BITS - 1);
            size_t bit_width = bit_end - bit_start + 1;

            atomic_t mask = (bit_width == ATOMIC_BITS) ? ~atomic_t(0) : ((atomic_t(1) << bit_width) - atomic_t(1));
            atomic_t v = (word >> bit_start) & mask;

            value |= (v << (bit_start + (index - start_index) * ATOMIC_BITS));
        }
    }

    void wr_flush()
    {
        backend_.wr_flush(); 
    }
    void rd_flush() 
    {
        backend_.rd_flush(); 
    }

private:
    Backend& backend_;
};
