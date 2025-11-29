# RTL emulator based on Kria KV260 evaluation board

This project demonstrates acceleration of LinkrunCCA blob-detection algorithm.
The RTL code is synthesized inside a wrapper, whcih allows to write all inputs to the DUT, fire a single clock pulse, and read all outputs from the DUT:

## Create FPGA project

Instructions for Linux based Vivado installation:

1. Open Vivad 2024.2

2. In TCL window, write command:<br>
`source ./fpga/src/tcl/create_vivado_project.tcl`

3. Generate bitstream.

4. File => Export => Export Hardware => Next => Select "Include bitstream" => Next => Use default filename and folder (accelerator_top & .../fpga_accelerated_sim/fpga_proj) => Next => Finish<br><br>
Alternatively write TCL command
`write_hw_platform -fixed -include_bit -force -file fpga_proj/accelerator_top.xsa`

5. Open terminal in the project root, and run command:<br>
`fpga/src/sh/extract_dtsi.sh`<br><br>
This will create two files to fpga_out/ folder. Those files are needed on Kria module.

## Compile C++ source code

TBD

## Copy needed files to Kria module

These files need to be copied to Kria:

- `fpga_out/accelerator_top.bit.bin`<br>
This is the bitstream of the accelerator to be loaded to Kria.
- `fpga_out/accelerator_top.dtbo`<br>
This is the device overlay, creating the `/dev/uio*` device.
- `build-arm64/fpga_app`<br>
This is the SW to be ran on the Kria module.

## Steps on Kria

Use ssh to log on to the Kria module.

### FPGA bitstream and overlay loading

This needs to be ran as root:

`fpgautil -R && sleep 1 && fpgautil -b accelerator_top.bit.bin  -o accelerator_top.dtbo`<br>
This will remove old overlay, and loads the new bitstream with the new overlay. The files need to reside in the same directory.

`echo 200000000 > /sys/devices/platform/fpga-region/fpga-region:clocking0/set_rate`<br>
This will change the clock speed exported from PS to PL to 200 MHz. Tested to work. 250 MHz is already too fast speed.

`time bash -c "./fpga_app -d /dev/uio4 | md5sum"`  Check from dmesg what is your uio device.<br>
This runs the application and checks the result against known results (not known, sorry). It should return `b801c5865a09c447291e70db5e7c4e35` in about 25.5 secs.

# Theory of operation

The emulator wrapper code to the DUT (vhdl) exposes a set of AXI4-Lite registers. The DUT input / ouptut signals are redirected starting at byte address 0x80.

On byte address 0x00, bit 0, is a clock advancing command. Writing value 1 to the register generates a single clock pulse for the DUT.

file `include/ver2/fields_linkruncca.h` lists the packing of inputs (wr_fields) to DUT, and outputs (rd_fields) from the DUT, and their respective bit_widths. Note that part of the bit widths are calculated using FPGA generics (X_SIZE and Y_BITS).

The FPGA code has same field definitions in `fpga/src/rtl/vhdl_linkruncca_pkg_ellipses_linescan.vhdl`. procedures `to_bits()` and `from_bits()` are used to serialize / deserialize structural data.

SW generates stimulus input by writing to `emulator_fields::wr_field()` function, which converts bit-sized writes to device specific (defined in `include/ver2/hw_access_aarch64.h` `wr_word_t` and `rd_word_t`) write-sizes. The data is first written to a shadow register, which are marked dirty upon writing. `emulator_fields::wr_flush()` writes all dirty words to the actual device, and marks all words in the shadow as non-dirty.

Read is the opposite. `emulator_fields::rd_flush()` marks all rd shadow entries as dirty, and upon calling `emulator_fields::rd_field()` the data is fetched from the real hardware, and corresponding data on shadow is marked as non-diry (so that it won't get re-read again).
