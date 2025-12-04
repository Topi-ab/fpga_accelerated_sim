#pragma once

#include <fcntl.h>
#include <sys/mman.h>

class hw_access_debug {
    public:
    using wr_word_t = uint32_t;
    using rd_word_t = uint8_t;

    static constexpr size_t first_wr_word_address = 0x10;
    static constexpr size_t first_rd_word_address = 0x10;

    hw_access_debug(const char *uio_dev) {
        // std::cout << "Opening UIO dev: " << uio_dev << "\n";
        fd_ = 1;
    }

    ~hw_access_debug()
    {
        std::cout << "Closing UIO dev:\n";
    }

    void wr_raw(size_t word_address, wr_word_t data) noexcept {
        // std::cout << "HW_ACCESS_DBG::wr_raw(" << word_address << ", " << data << ");\n";
        wr_space[word_address] = data;
    }

    void wr(size_t word_offset, wr_word_t data) noexcept {
        // std::cout << "HW_ACCESS_DBG::wr(" << word_offset << ", " << data << ");\n";
        wr_raw(first_wr_word_address + word_offset * wr_word_bytes, data);
    }

    rd_word_t rd_raw(size_t word_address) noexcept {
        return rd_space[word_address];
    }

    rd_word_t rd(size_t word_offset) noexcept {
        return rd_raw(first_rd_word_address + word_offset * rd_word_bytes);
    }
private:
    int     fd_ = -1;
    std::array<wr_word_t, 1024>  wr_space;
    std::array<rd_word_t, 1024>  rd_space;

    static constexpr size_t wr_word_bytes =sizeof(wr_word_t);
    static constexpr size_t rd_word_bytes =sizeof(wr_word_t);
    
    size_t  map_size_ = 0;
};
