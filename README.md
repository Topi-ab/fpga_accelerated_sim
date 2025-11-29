# RTL Emulator Based on the Kria KV260 Evaluation Board

## Supported Linux versions on Kria

This project expects the Ubuntu overlay-enabled Linux distribution for Kria, i.e. a system that supports loading FPGA bitstreams and device-tree overlays using `fpgautil`. Verified environment:

```
$ sb_release -a
No LSB modules are available.
Distributor ID:	Ubuntu
Description:	Ubuntu 24.04.3 LTS
Release:	24.04
Codename:	noble
```

### Needed packages (incomplete)

```
sudo apt install fpga-manager-xlnx device-tree-compiler xrt
```

## Project scope

This project demonstrates FPGA-based acceleration of the LinkrunCCA blob-detection algorithm.
The RTL code is wrapped in an emulator shell that allows software to:

- write all DUT input signals,
- issue a single clock pulse, and
- read all DUT output signals.

# FPGA Project Creation

Instructions assume Vivado 2024.2 on Linux.

1. Open Vivado 2024.2.

2. In the TCL console, run:
   source ./fpga/src/tcl/create_vivado_project.tcl

3. Run synthesis/implementation and generate the bitstream.

4. Export the hardware platform.

   GUI:
     File → Export → Export Hardware → Next → Include bitstream → Next → Finish

   CLI:
     write_hw_platform -fixed -include_bit -force -file fpga_proj/accelerator_top.xsa

5. Extract device tree overlay:
   fpga/src/sh/extract_dtsi.sh

This produces two files under fpga_out/, which must be copied to the Kria module.

# Compile the C++ Application

TBD

# Files to Copy to the Kria Module

- fpga_out/accelerator_top.bit.bin  
  FPGA bitstream to be loaded on Kria.

- fpga_out/accelerator_top.dtbo  
  Device tree overlay creating the /dev/uio* interface.

- build-arm64/fpga_app  
  Executable to run on the Kria module.

# Steps on the Kria Module

Log in via SSH.

## Load FPGA Bitstream and Overlay (as root)

fpgautil -R && sleep 1 && \
fpgautil -b accelerator_top.bit.bin -o accelerator_top.dtbo

The .bit.bin and .dtbo files must be in the current directory.

## Set PL Clock Frequency

echo 200000000 > /sys/devices/platform/fpga-region/fpga-region:clocking0/set_rate

This sets the PL clock to 200 MHz.

250 MHz is already too fast.

## Run the Application

Find your /dev/uioX from dmesg, then run:

time bash -c "./fpga_app -d /dev/uio4 | md5sum"<br>
replace `/dev/uio4` with the uio device you got.

Expected checksum:
b801c5865a09c447291e70db5e7c4e35  
Runtime: ~25.5 seconds.

# Theory of Operation

The RTL emulator exposes a set of AXI4-Lite registers.

- DUT input and output fields begin at **byte address 0x80**.
- Address **0x00**, bit **0** → writing `1` generates a **single DUT clock pulse**.

Field packing for DUT inputs (wr_fields) and outputs (rd_fields) is defined in:
  include/ver2/fields_linkruncca.h

Bit widths depend on generics (X_SIZE, Y_BITS).

Matching FPGA-side field definitions are in:
  fpga/src/rtl/vhdl_linkruncca_pkg_ellipses_linescan.vhdl

Serialization/deserialization uses:
  to_bits()
  from_bits()

## Software Access Layer

Writing:
  emulator_fields::wr_field()
    - Packs field writes into device-specific words.
    - Writes into a shadow register and marks it dirty.

  emulator_fields::wr_flush()
    - Writes all dirty shadow entries to hardware.
    - Clears dirty flags.

Reading:
  emulator_fields::rd_flush()
    - Marks all read shadow entries dirty.

  emulator_fields::rd_field()
    - Reads from hardware into shadow registers as needed.
    - Clears dirty flags.

This minimizes redundant AXI-Lite reads/writes and keeps access efficient.
