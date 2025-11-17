#pragma once
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>
#include <ostream>

// ============================================================================
//  DegulBackend
//  - arbitrary read response backend for debugging
//  - dumps API calls to stdout
// ============================================================================
//  - write atomic size = 128 bits
//  - read  atomic size = 64 bits
//  - backend owns WR and RD shadow buffers + dirty flags
//  - upper layer provides total WR/RD bit sizes via init()
// ============================================================================

std::ostream& operator<<(std::ostream& os, unsigned __int128 value)
{
    char buf[33];   // 32 hex digits + null terminator
    int i = 32;
    buf[i--] = '\0';

    if (value == 0) {
        os << "0";
        return os;
    }

    while (value > 0 && i >= 0) {
        const int digit = value & 0xF;
        buf[i--] = "0123456789abcdef"[digit];
        value >>= 4;
    }

    return os << (buf + i + 1);
}

class DebugBackend {
public:
    // using write_word_t = __uint128_t;   // 128-bit shadow + store
    // using read_word_t  = uint64_t;      // 64-bit shadow + load
    using write_word_t = __uint16_t;   // 128-bit shadow + store
    using read_word_t  = uint32_t;      // 64-bit shadow + load

    // ------------------------------------------------------------
    // Constructor: only maps MMIO, does not allocate shadows
    // ------------------------------------------------------------
    DebugBackend()
    {
        std::cout << "BACKEND: DebugBackend: open\n";
    }

    // ------------------------------------------------------------
    // Destructor: unmap + close
    // ------------------------------------------------------------
    ~DebugBackend()
    {
        std::cout << "BACKEND: DebugBackend: closed\n";
    }

    // ------------------------------------------------------------
    // init: must be called AFTER constructor
    //       upper layer provides real packed sizes
    // ------------------------------------------------------------
    void init(std::size_t wr_bits, std::size_t rd_bits)
    {
        std::cout << "BACKEND: DebugBackend: init(wr_bits=" << wr_bits
                  << ", rd_bits=" << rd_bits << ")\n";
        // compute number of atomic words
        wr_words_ = (wr_bits + (sizeof(write_word_t)*8 - 1)) 
                    / (sizeof(write_word_t)*8);
        rd_words_ = (rd_bits + (sizeof(read_word_t)*8 - 1)) 
                    / (sizeof(read_word_t)*8);

        // allocate wr shadow + dirty flags
        wr_shadow_ = new write_word_t[wr_words_];
        wr_dirty_  = new bool[wr_words_];
        for (std::size_t i = 0; i < wr_words_; i++) {
            wr_shadow_[i] = 0;
            wr_dirty_[i] = false;   // clean initially
        }

        // allocate rd shadow + dirty flags
        rd_shadow_ = new read_word_t[rd_words_];
        rd_dirty_  = new bool[rd_words_];
        for (std::size_t i = 0; i < rd_words_; i++) {
            rd_dirty_[i] = true;     // unread initially
        }
    }

    // ------------------------------------------------------------
    // WR WRITE:
    //   shadow[word] = merge(shadow, wr_data, wr_mask)
    //   marks shadow dirty
    // ------------------------------------------------------------
    void wr_write(std::size_t wr_word_index,
                  write_word_t wr_data,
                  write_word_t wr_mask)
    {
        std::cout << "BACKEND: DebugBackend: WR WRITE word " << wr_word_index
                  << " data=0x" << std::hex << wr_data
                  << " mask=0x" << wr_mask << std::dec << "\n";
        auto new_data = (wr_shadow_[wr_word_index] & ~wr_mask) | (wr_data & wr_mask);
        auto& dirty = wr_dirty_[wr_word_index];
        write_word_t& dst = wr_shadow_[wr_word_index];
        if(!dirty || dst != new_data) {
            std::cout << "BACKEND: DebugBackend: Index " << wr_word_index << " marked dirty\n";
            dst = (dst & ~wr_mask) | (wr_data & wr_mask);
            // wr_dirty_[wr_word_index] = true;
            dirty = true;
        }
    }

    // ------------------------------------------------------------
    // WR FLUSH:
    //   write all dirty 128-bit words to HW
    // ------------------------------------------------------------
    void wr_flush()
    {
        for (std::size_t i = 0; i < wr_words_; i++) {
            auto& dirty = wr_dirty_[i];
            if (dirty) {
                std::cout << "BACKEND: DebugBackend: WR FLUSH word " << i
                      << " data=0x" << std::hex << wr_shadow_[i] << std::dec << "\n";
                dirty = false;
            }
        }
    }

    // ------------------------------------------------------------
    // RD READ:
    //   if dirty → read from HW → fill shadow → clear dirty
    //   return shadow
    // ------------------------------------------------------------
    read_word_t rd_read(std::size_t rd_word_index)
    {
        std::cout << "BACKEND: DebugBackend: RD READ word " << rd_word_index << "\n";
        auto& dirty = rd_dirty_[rd_word_index];
        if (dirty) {
            // simulate read from HW: fill with dummy data
            std::cout << "BACKEND: DebugBackend: rd_read() word index: " << rd_word_index << " (fetching from HW)\n";
            rd_shadow_[rd_word_index] = static_cast<read_word_t>(rd_word_index + 3);
            dirty = false;
        }
        std::cout << "BACKEND: DebugBackend: rd_read() returning data=0x" << std::hex << rd_shadow_[rd_word_index] << std::dec << "\n";
        return rd_shadow_[rd_word_index];

        /*if (rd_dirty_[rd_word_index]) {
            auto* base = reinterpret_cast<volatile read_word_t*>(mmio_);
            rd_shadow_[rd_word_index] = base[rd_word_index];
            rd_dirty_[rd_word_index] = false;
        }
        return rd_shadow_[rd_word_index];*/
    }

    // ------------------------------------------------------------
    // RD FLUSH:
    //   mark ALL rd shadow dirty → next rd_read fetches from HW
    // ------------------------------------------------------------
    void rd_flush()
    {
        for (std::size_t i = 0; i < rd_words_; i++)
            rd_dirty_[i] = true;
    }

private:
    // shadow memory
    write_word_t* wr_shadow_ = nullptr;
    bool*         wr_dirty_  = nullptr;
    std::size_t   wr_words_  = 0;

    read_word_t*  rd_shadow_ = nullptr;
    bool*         rd_dirty_  = nullptr;
    std::size_t   rd_words_  = 0;
};

