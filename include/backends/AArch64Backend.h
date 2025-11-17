#pragma once
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

// ============================================================================
//  AArch64Backend
//  - MMIO mapping for /dev/uioX
//  - write atomic size = 128 bits
//  - read  atomic size = 64 bits
//  - backend owns WR and RD shadow buffers + dirty flags
//  - upper layer provides total WR/RD bit sizes via init()
// ============================================================================

class AArch64Backend {
public:
    using write_word_t = __uint128_t;   // 128-bit shadow + store
    using read_word_t  = uint64_t;      // 64-bit shadow + load

    // ------------------------------------------------------------
    // Constructor: only maps MMIO, does not allocate shadows
    // ------------------------------------------------------------
    AArch64Backend(const char* uio_dev, int map_index)
    {
        fd_ = ::open(uio_dev, O_RDWR | O_SYNC);
        if (fd_ < 0)
            throw std::runtime_error("Failed to open UIO device");

        // resolve uio name
        const char* base = strrchr(uio_dev, '/');
        const char* name = base ? base + 1 : uio_dev;

        char path[256];
        snprintf(path, sizeof(path),
                 "/sys/class/uio/%s/maps/map%d/size",
                 name, map_index);

        // read map size
        FILE* f = fopen(path, "r");
        if (!f)
            throw std::runtime_error("Cannot read UIO map size");
        if (fscanf(f, "%zi", &map_size_) != 1) {
            fclose(f);
            throw std::runtime_error("Invalid UIO map size");
        }
        fclose(f);

        // mmap the region
        mmio_ = mmap(nullptr, map_size_,
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED,
                     fd_,
                     map_index * getpagesize());

        if (mmio_ == MAP_FAILED)
            throw std::runtime_error("mmap() failed");
    }

    // ------------------------------------------------------------
    // Destructor: unmap + close
    // ------------------------------------------------------------
    ~AArch64Backend()
    {
        if (mmio_ && mmio_ != MAP_FAILED)
            munmap(mmio_, map_size_);
        if (fd_ >= 0)
            close(fd_);
    }

    // ------------------------------------------------------------
    // init: must be called AFTER constructor
    //       upper layer provides real packed sizes
    // ------------------------------------------------------------
    void init(std::size_t wr_bits, std::size_t rd_bits)
    {
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
        write_word_t& dst = wr_shadow_[wr_word_index];
        dst = (dst & ~wr_mask) | (wr_data & wr_mask);
        wr_dirty_[wr_word_index] = true;
    }

    // ------------------------------------------------------------
    // WR FLUSH:
    //   write all dirty 128-bit words to HW
    // ------------------------------------------------------------
    void wr_flush()
    {
        auto* base = reinterpret_cast<volatile write_word_t*>(mmio_);
        for (std::size_t i = 0; i < wr_words_; i++) {
            if (wr_dirty_[i]) {
                base[i] = wr_shadow_[i];
                wr_dirty_[i] = false;
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
        if (rd_dirty_[rd_word_index]) {
            auto* base = reinterpret_cast<volatile read_word_t*>(mmio_);
            rd_shadow_[rd_word_index] = base[rd_word_index];
            rd_dirty_[rd_word_index] = false;
        }
        return rd_shadow_[rd_word_index];
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
    // MMIO
    int     fd_ = -1;
    void*   mmio_ = nullptr;
    size_t  map_size_ = 0;

    // shadow memory
    write_word_t* wr_shadow_ = nullptr;
    bool*         wr_dirty_  = nullptr;
    std::size_t   wr_words_  = 0;

    read_word_t*  rd_shadow_ = nullptr;
    bool*         rd_dirty_  = nullptr;
    std::size_t   rd_words_  = 0;
};

