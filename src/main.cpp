#include <iostream>

#include "FpgaInterface/FpgaGenerics.h"

constexpr FpgaGenerics generics(65535, 16);



// Include your backend and interface:

#include <backends/AArch64Backend.h>
#include <backends/DebugBackend.h>
#include <PacketStructAccessor/PacketStructAccessor.h>
#include <FpgaInterface/FpgaInterface_LinkRunCCA.h>

// const char* dev_fname = "/dev/uio0";
const char* dev_fname = "file.bin";


int main()
{
    using wr_add = LinkRunCCA_IF<generics>::LinkRunCCA_collect_t_fields;
    using rd_add = LinkRunCCA_IF<generics>::LinkRunCCA_feature_t_fields;

    std::cout << "FPGA Interface Test" << std::endl;

    // -------------------------------------------------------------
    // 1. Construct backend (example arguments)
    // -------------------------------------------------------------

    // AArch64Backend backend(dev_fname, 0);
    DebugBackend backend;

    // Init backend with some example bit sizes.
    // These should match your VHDL layout.

    /*const size_t wr_bits = 512;   // example
    const size_t rd_bits = 512;   // example
    backend.init(wr_bits, rd_bits);*/

    // -------------------------------------------------------------
    // 2. Construct FPGA interface
    // -------------------------------------------------------------

    FpgaInterface<decltype(backend), LinkRunCCA_IF<generics> > iface(backend);

    // -------------------------------------------------------------
    // 3. Do example write (depends on how you define your IF)
    // -------------------------------------------------------------

    std::cout << "Writing dummy values…" << std::endl;

    iface.write(wr_add::IN_LABEL, 1);
    iface.write(wr_add::X, 255);
    iface.write(wr_add::Y, 456);
    iface.write(wr_add::HAS_BLUE, 1);

    iface.wr_flush();

    // -------------------------------------------------------------
    // 4. Example read
    // -------------------------------------------------------------

    iface.rd_flush();

    uint32_t feature;

    iface.read(rd_add::X_LEFT, feature);
    std::cout << "Read X_LEFT: " << feature << std::endl;

    uint64_t n_seg1_sum;
    
    iface.read(rd_add::N_SEG1_SUM, n_seg1_sum);
    std::cout << "Read N_SEG1_SUM: " << n_seg1_sum << std::endl;


    return 0;
}
