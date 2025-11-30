#pragma once

template <typename shadow_t>
class bit_slicer {
public:
    bit_slicer(shadow_t &sh) : shadow_(sh) {}

    template<typename word_t>
    inline void write_bits(size_t bit_offset, size_t bit_width, word_t data) {
        using atomic_t = typename shadow_t::wr_word_t;
        static constexpr size_t ATOMIC_BITS = sizeof(atomic_t) * 8;

        if (bit_width == 0)
            return; // or throw

        auto begin_idx = bit_offset / ATOMIC_BITS;
        auto end_idx   = (bit_offset + bit_width - 1) / ATOMIC_BITS;

        for (auto idx = begin_idx; idx <= end_idx; ++idx) {
            size_t bit_begin = (idx == begin_idx) ? (bit_offset % ATOMIC_BITS) : 0;
            size_t bit_end   = (idx == end_idx)
                            ? ((bit_offset + bit_width - 1) % ATOMIC_BITS)
                            : (ATOMIC_BITS - 1);

            size_t wr_bits = bit_end - bit_begin + 1;  // bits written in THIS word

            atomic_t mask = (wr_bits == ATOMIC_BITS)
                        ? ~atomic_t(0)
                        : ((atomic_t(1) << wr_bits) - 1);

            atomic_t v = static_cast<atomic_t>((data) & mask);

            atomic_t wr_mask = mask << bit_begin;
            atomic_t wr_word = v    << bit_begin;

            shadow_.write(idx, wr_word, wr_mask);

            data >>= wr_bits;
        }
    }

    inline void wr_hw(size_t word_address, typename shadow_t::wr_word_t data) {
        shadow_.wr_hw(word_address, data);
    }

    template<typename word_t>
    inline word_t read_bits(size_t bit_offset, size_t bit_width) {
        using atomic_t = typename shadow_t::rd_word_t;
        static constexpr size_t ATOMIC_BITS = sizeof(atomic_t) * 8;

        if (bit_width == 0)
            throw std::runtime_error("read_bits: width of zero not allowed");

        if (bit_width > sizeof(word_t) * 8)
            throw std::runtime_error("read_bits: reading field wider than word_t");

        word_t r = 0;

        auto begin_idx = bit_offset / ATOMIC_BITS;
        auto end_idx   = (bit_offset + bit_width - 1) / ATOMIC_BITS;

        for (auto idx = begin_idx; idx <= end_idx; ++idx) {
            atomic_t word = shadow_.read(idx);

            size_t bit_begin = (idx == begin_idx) ? (bit_offset % ATOMIC_BITS) : 0;
            size_t bit_end   = (idx == end_idx)
                            ? ((bit_offset + bit_width - 1) % ATOMIC_BITS)
                            : (ATOMIC_BITS - 1);
            size_t rd_bits   = bit_end - bit_begin + 1;

            atomic_t mask = (rd_bits == ATOMIC_BITS)
                ? ~atomic_t(0)
                : ((atomic_t(1) << rd_bits) - 1);

            atomic_t rd_word = (word >> bit_begin) & mask;

            // Pack into the result starting at dst_pos
            size_t abs_bit_pos = idx * ATOMIC_BITS + bit_begin;
            size_t dst_pos = abs_bit_pos - bit_offset;            
            r |= word_t(rd_word) << dst_pos;
        }

        return r;
    }

    inline typename shadow_t::rd_word_t rd_hw(size_t word_address) {
        return shadow_.rd_hw(word_address);
    }

    inline void wr_flush() {
        shadow_.wr_flush();
    }

    inline void rd_flush() {
        shadow_.rd_flush();
    }
private:
    shadow_t &shadow_;
};
