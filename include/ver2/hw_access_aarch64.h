#pragma once

#include <fcntl.h>
#include <sys/mman.h>

class hw_access_aarch64 {
    public:
    // using wr_word_t = uint64_t;
    // using rd_word_t = uint64_t;
    using wr_word_t = __uint128_t;
    using rd_word_t = __uint128_t;

    hw_access_aarch64(const char *uio_dev) {
        fd_ = ::open(uio_dev, O_RDWR | O_SYNC);
        if (fd_ < 0)
            throw std::runtime_error("Failed to open UIO device");

        const char* base = strrchr(uio_dev, '/');
        const char* name = base ? base + 1 : uio_dev;

        char path[256];
        int map_index = 0;
        snprintf(path, sizeof(path),
            "/sys/class/uio/%s/maps/map%d/size",
            name, map_index);

        FILE* f = fopen(path, "r");
        if (!f)
            throw std::runtime_error("Cannot read UIO map size");
        if (fscanf(f, "%zx", &map_size_) != 1) {
            fclose(f);
            throw std::runtime_error("Invalid UIO map size");
        }
        fclose(f);
        
        mmio_ = mmap(nullptr, map_size_,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd_,
            map_index * getpagesize());

        if (mmio_ == MAP_FAILED)
            throw std::runtime_error("mmap() failed");
    }

    ~hw_access_aarch64()
    {
        if (mmio_ && mmio_ != MAP_FAILED)
            munmap(mmio_, map_size_);
        if (fd_ >= 0)
            close(fd_);
    }

    void wr_raw(size_t word_address, wr_word_t data) noexcept {
        auto* base = reinterpret_cast<volatile wr_word_t*>(mmio_);

        if constexpr (sizeof(wr_word_t)*8 == 128) {
            volatile void *p = static_cast<volatile uint8_t*>(reinterpret_cast<volatile void *>(base + word_address));
            const uint64_t *v = reinterpret_cast<const uint64_t *>(& data);
            store128(p, v[0], v[1]);
        }
        else {
            base[word_address] = data;
        }
    }

    void wr(size_t word_offset, wr_word_t data) noexcept {
        wr_raw(first_wr_word_address + word_offset, data);
    }

    rd_word_t rd_raw(size_t word_address) noexcept {
        auto volatile * base = reinterpret_cast<volatile rd_word_t*>(mmio_);

        if constexpr (sizeof(rd_word_t)*8 == 128) {
            volatile void *p = reinterpret_cast<volatile void *>(base + word_address);
            rd_word_t data;
            uint64_t *v = reinterpret_cast<uint64_t *>(&data);
            load128(p, v[0], v[1]);
            /*if(data != 0)
                std::cout << "read: " << v[0] << v[1] << "\n";*/
            return data;
        }
        else {
            return base[word_address];
        }
    }

    rd_word_t rd(size_t word_offset) noexcept {
        return rd_raw(first_rd_word_address + word_offset);
    }
private:
    static inline void store128(volatile void *ptr, uint64_t lo, uint64_t hi)
    {
        __asm__ volatile(
            "stp %x0, %x1, [%x2]"
            :
            : "r"(lo), "r"(hi), "r"(ptr)
            : "memory"
        );
    }

    static inline void load128(const volatile void *ptr, uint64_t &lo, uint64_t &hi)
    {
        __asm__ volatile(
            "ldp %x0, %x1, [%x2]"
            : "=r"(lo), "=r"(hi)
            : "r"(ptr)
            : "memory"
        );
    }

    static constexpr size_t wr_word_bytes =sizeof(wr_word_t);
    static constexpr size_t rd_word_bytes =sizeof(rd_word_t);
    
    static constexpr size_t first_wr_byte_address = 0x80;
    static constexpr size_t first_rd_byte_address = 0x80;

    static constexpr size_t first_wr_word_address = first_wr_byte_address / wr_word_bytes;
    static constexpr size_t first_rd_word_address = first_rd_byte_address / rd_word_bytes;

    int     fd_ = -1;
    void*   mmio_ = nullptr;
    size_t  map_size_ = 0;
};
