create_clock -period 1000.000 -name DUT_CLK -waveform {0.000 2.000} [get_nets emulator_top_i/dut_clk]

set_false_path -through [get_pins -of_objects [get_cells emulator_top_i/dut]]

