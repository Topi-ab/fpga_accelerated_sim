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
