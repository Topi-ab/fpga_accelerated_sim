#pragma once
#include <cstddef>
#include <cstdint>
#include <stdexcept>

// template <typename Backend, typename word_t = uint64_t>
template <typename Backend>
class PackedStructAccessor {
public:
    // using word_t = uint64_t;  // both rd and wr words are 64-bit

    // static constexpr size_t ATOMIC_BITS = sizeof(word_t) * 8;

    PackedStructAccessor(Backend& backend,
                         size_t wr_bits,
                         size_t rd_bits)
        : backend_(backend)
    {
        backend_.init(wr_bits, rd_bits);
    }

    // ------------------------------------------------------------
    // WRITE FIELD (<=64 bits, must not cross 64-bit boundary)
    // ------------------------------------------------------------
    template <typename word_t = uint32_t>
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

        /*
        if (width == 0 || width > ATOMIC_BITS)
            throw std::runtime_error("write_bits: invalid width");

        //word_t mask = (width == 64) ? ~0ULL : ((1ULL << width) - 1ULL);
        word_t mask = (width == ATOMIC_BITS) ? ~word_t(0) : ((word_t(1) << width) - word_t(1));
        value &= mask;

        size_t word = offset / ATOMIC_BITS;
        size_t bit  = offset % ATOMIC_BITS;

        if (bit + width > ATOMIC_BITS)
            throw std::runtime_error("write_bits: field crosses word boundary");

        word_t wr_mask = mask << bit;
        word_t wr_data = value << bit;

        backend_.wr_write(word, wr_data, wr_mask);*/

    }

    // ------------------------------------------------------------
    // READ FIELD (<=64 bits, must not cross 64-bit boundary)
    // ------------------------------------------------------------
    template <typename word_t>
    void read_bits(size_t offset, size_t width, word_t& value)
    {
        using atomic_t = typename Backend::read_word_t;
        //static constexpr size_t ATOMIC_BITS = sizeof(word_t) * 8;
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

        /*
        if (width == 0 || width > ATOMIC_BITS)
            throw std::runtime_error("read_bits: invalid width");

        size_t word = offset / ATOMIC_BITS;
        size_t bit  = offset % ATOMIC_BITS;

        if (bit + width > ATOMIC_BITS)
            throw std::runtime_error("read_bits: field crosses word boundary");

        word_t w = backend_.rd_read(word);
        word_t v = w >> bit;
        
        word_t mask = (width == ATOMIC_BITS) ? ~word_t(0) : ((word_t(1) << width) - word_t(1));
        value = v & mask;
        */
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
