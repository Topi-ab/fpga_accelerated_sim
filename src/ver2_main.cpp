#include <vector>
#include <variant>
#include <memory>

#include <iostream>

#include "ver2/FpgaGenerics.h"

#include <boost/multiprecision/cpp_int.hpp>
using boost::multiprecision::cpp_int;

constexpr FpgaGenerics generics(65535, 16);


#define DEBUG_PRINT


// Include your backend and interface:

/*#include <backends/AArch64Backend.h>
#include <backends/DebugBackend.h>
#include <PacketStructAccessor/PacketStructAccessor.h>
#include <FpgaInterface/FpgaInterface_LinkRunCCA.h>*/

#include <ver2/shadow.h>
#include <ver2/fields_linkruncca.h>
#include <ver2/hw_access_aarch64.h>
#include <ver2/hw_access_debug.h>
// #include <ver2/shadow.h> !!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <ver2/bit_slicer.h>
#include <ver2/fields.h>
#include <ver2/emulator_fields.h>



#if defined(__aarch64__)
using BackendType = hw_access_aarch64;
#else
using BackendType = hw_access_debug;
// using BackendType = hw_access_aarch64;
#endif

constexpr FpgaGenerics_linkruncca llcca_gens{
    .X_SIZE=1024,
    .Y_BITS=16,
};

using app_fields_t = fields_linkruncca<llcca_gens>;

using emulator_t = emulator_fields<BackendType, app_fields_t>;

using wr_add = typename emulator_t::wr_fields;
using rd_add = typename emulator_t::rd_fields;




const char* dev_fname = "/dev/uio4";
// const char* dev_fname = "file.bin";
enum ObjectType {
    CIRCLE,
    SQUARE,
};

struct ObjectBase {
    virtual ~ObjectBase() = default;
    virtual bool pixel(size_t x, size_t y) const = 0;
};

struct CircleData: ObjectBase {
    size_t x;
    size_t y;
    size_t radius;
    CircleData(size_t x_, size_t y_, size_t radius_) : x(x_), y(y_), radius(radius_) {}
    bool pixel(size_t x, size_t y) const override {
        long dx = static_cast<long>(x) - static_cast<long>(this->x);
        long dy = static_cast<long>(y) - static_cast<long>(this->y);
        long r = static_cast<long>(radius);
        return (dx*dx + dy*dy) <= (r * r);
    }
};

struct SquareData: ObjectBase {
    size_t x;
    size_t y;
    size_t side_length;
    SquareData(size_t x_, size_t y_, size_t side_length_) : x(x_), y(y_), side_length(side_length_) {}
    bool pixel(size_t x, size_t y) const override {
        return (x >= this->x && x < this->x + side_length &&
                y >= this->y && y < this->y + side_length);
    }
};

struct Collect_t {
    bool in_label;
    size_t x;
    size_t y;
    bool has_red;
    bool has_green;
    bool has_blue;
};

struct Feature_t {
    bool valid;
    size_t x_left;
    size_t x_right;
    size_t y_top_seg_0;
    size_t y_top_seg_1;
    size_t y_bottom_seg_0;
    size_t y_bottom_seg_1;
    cpp_int x2_sum;
    cpp_int ylow2_sum;
    cpp_int xylow_sum;
    cpp_int x_seg0_sum;
    cpp_int x_seg1_sum;
    cpp_int ylow_seg0_sum;
    cpp_int ylow_seg1_sum;
    cpp_int n_seg0_sum;
    cpp_int n_seg1_sum;
};

class TestFrame {
public:
    using Objects = std::vector<std::unique_ptr<ObjectBase>>;
private:
    Objects objects_;

public:
    TestFrame(Objects objects)
        : objects_(std::move(objects))
    {}

    Collect_t GetPixel(size_t x, size_t y, size_t repeat_y) const {
        for(const auto& obj : objects_) {
            if(obj->pixel(x, y % repeat_y)) {
                return Collect_t{true, x, y, false, false, false};
            }
        }
        return Collect_t{false, x, y, false, false, false};
    }
};

class TestFrames {
private:
    std::vector<TestFrame> frames_;
public:
    TestFrames(size_t x_size, size_t y_size, size_t repeat_y_size)
        : x_size(x_size), y_size(y_size), repeat_y(repeat_y_size)
    {
        // Create some test frames with objects
        for(size_t f = 0; f < 10; ++f) {
            TestFrame::Objects objects;
            objects.push_back(std::make_unique<CircleData>(20 + f*40, 20 + f*3, 12));
            objects.push_back(std::make_unique<SquareData>(30 + f*30,  30 + f*2, 10));
            frames_.emplace_back(std::move(objects));
        }
    }

    TestFrame& GetFrame(size_t index) {
        return frames_[index % frames_.size()];
    }

    Collect_t GetPixel(size_t frame_index, size_t x, size_t y) const {
        return frames_[frame_index % frames_.size()].GetPixel(x, y, repeat_y);
    }

    const size_t x_size;
    const size_t y_size;
    const size_t repeat_y;
};

void WrEmulationData(emulator_t &iface, Collect_t &data) {
    iface.wr_field(wr_add::RST, 0);
    iface.wr_field(wr_add::DATAVALID, 1);
    iface.wr_field(wr_add::IN_LABEL, data.in_label ? 1 : 0);
    iface.wr_field(wr_add::X, data.x);
    iface.wr_field(wr_add::Y, data.y);
    iface.wr_field(wr_add::HAS_RED, data.has_red ? 1 : 0);
    iface.wr_field(wr_add::HAS_GREEN, data.has_green ? 1 : 0);
    iface.wr_field(wr_add::HAS_BLUE, data.has_blue ? 1 : 0);

    iface.wr_flush();
    iface.wr_raw(0, (uint32_t)1);
}

bool RdEmulationData(emulator_t &iface, Feature_t &data) {
    iface.rd_flush();
    iface.rd_field(rd_add::VALID, data.valid);
    if(data.valid) {
        iface.rd_field(rd_add::X_LEFT, data.x_left);
        iface.rd_field(rd_add::X_RIGHT, data.x_right);
        iface.rd_field(rd_add::Y_TOP_SEG_0, data.y_top_seg_0);
        iface.rd_field(rd_add::Y_TOP_SEG_1, data.y_top_seg_1);
        iface.rd_field(rd_add::Y_BOTTOM_SEG_0, data.y_bottom_seg_0);
        iface.rd_field(rd_add::Y_BOTTOM_SEG_1, data.y_bottom_seg_1);
        iface.rd_field(rd_add::X2_SUM, data.x2_sum);
        iface.rd_field(rd_add::YLOW2_SUM, data.ylow2_sum);
        iface.rd_field(rd_add::XYLOW_SUM, data.xylow_sum);
        iface.rd_field(rd_add::X_SEG0_SUM, data.x_seg0_sum);
        iface.rd_field(rd_add::X_SEG1_SUM, data.x_seg1_sum);
        iface.rd_field(rd_add::YLOW_SEG0_SUM, data.ylow_seg0_sum);
        iface.rd_field(rd_add::YLOW_SEG1_SUM, data.ylow_seg1_sum);
        iface.rd_field(rd_add::N_SEG0_SUM, data.n_seg0_sum);
        iface.rd_field(rd_add::N_SEG1_SUM, data.n_seg1_sum);
    }
    return data.valid;
}

void TestRun(emulator_t &iface) {
    const size_t frames = 1;
    const size_t x_size = llcca_gens.X_SIZE;
    const size_t y_bits = llcca_gens.Y_BITS;
    const size_t y_size = (size_t)1 << y_bits;
    const size_t repeat_y_size = 512;
    const size_t max_clk_cnt = 50000000;

    TestFrames test_frames(x_size, y_size, repeat_y_size);

    iface.wr_field(wr_add::RST, 1);
    iface.wr_field(wr_add::DATAVALID, 1);
    iface.wr_flush();
    for(int i=0; i < 2*x_size; i++)
        iface.wr_raw(0, (uint32_t)1);
    iface.wr_field(wr_add::RST, 0);
    iface.wr_field(wr_add::DATAVALID, 0);
    iface.wr_flush();
    iface.wr_raw(0, (uint32_t)1);

    uint64_t clk_cnt = 0;
    for(size_t frame_idx = 0; frame_idx < frames; ++frame_idx) {
#ifdef DEBUG_PRINT
        std::cout << "Frame " << frame_idx << ":\n";
#endif
        for(auto y = 0; y < y_size; ++y) {
            for(size_t x = 0; x < x_size; ++x) {
                Collect_t pixel = test_frames.GetPixel(frame_idx, x, y);
                Feature_t feature;
                WrEmulationData(iface, pixel);
                clk_cnt++;
                if(RdEmulationData(iface, feature)) {
                    std::cout << "FEATURE:";
                    std::cout << "\n  clk_cnt = " << clk_cnt << "\n  X_LEFT: " << feature.x_left << "\n  X_RIGHT: " << feature.x_right;
                    std::cout << "\n  y_top_seg_0 = " << feature.y_top_seg_0 << "\n  y_bottom_seg_0 = " << feature.y_bottom_seg_0;
                    std::cout << "\n  y_top_seg_1 = " << feature.y_top_seg_1 << "\n  y_bottom_seg_1 = " << feature.y_bottom_seg_1;

                    std::cout << "\n\n";
                }
                if(clk_cnt >= max_clk_cnt)
                    break;
            }
            if(clk_cnt >= max_clk_cnt)
                break;
        }
    }

    std::cout << "Emulation ended\n\n Processed " << clk_cnt << " clock cycles.\n\n";
}



int main()
{
#ifdef DEBUG_PRINT
    std::cout << "FPGA Interface Test" << std::endl;
#endif


    BackendType hw("/dev/uio4");

    emulator_t emulator(hw);

    if(true) {
        TestRun(emulator);

        return 0;
    }

    emulator.wr_field(wr_add::IN_LABEL, 1);
    emulator.wr_flush();
    std::cout << "Flushed\n";
    emulator.wr_field(wr_add::HAS_BLUE, 1);

    emulator.wr_flush();

    return 0;
}
