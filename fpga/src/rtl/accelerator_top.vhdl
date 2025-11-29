library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.vhdl_linkruncca_pkg.all;

entity accelerator_top is
end;

architecture rtl of accelerator_top is
    constant AXI_DATA_BITS: positive := 64;

    signal clk_250m: std_logic;
    signal clk_250m_sresetn: std_logic;
    signal clk_250m_sreset: std_logic;

    signal axil_awready: std_logic;
    signal axil_awvalid: std_logic;
    signal axil_awprot: std_logic_vector(2 downto 0);
    signal axil_awaddr: std_logic_vector(15 downto 0);

    signal axil_wready: std_logic;
    signal axil_wvalid: std_logic;
    signal axil_wstrb: std_logic_vector(AXI_DATA_BITS/8-1 downto 0);
    signal axil_wdata: std_logic_vector(AXI_DATA_BITS-1 downto 0);

    signal axil_bready: std_logic;
    signal axil_bvalid: std_logic;
    signal axil_bresp: std_logic_vector(1 downto 0);

    signal axil_arready: std_logic;
    signal axil_arvalid: std_logic;
    signal axil_arprot: std_logic_vector(2 downto 0);
    signal axil_araddr: std_logic_vector(15 downto 0);

    signal axil_rready: std_logic;
    signal axil_rvalid: std_logic;
    signal axil_rresp: std_logic_vector(1 downto 0);
    signal axil_rdata: std_logic_vector(AXI_DATA_BITS-1 downto 0);
    
    attribute DONT_TOUCH: string;
    
    attribute DONT_TOUCH of axil_wstrb: signal is "TRUE";
    attribute DONT_TOUCH of axil_awprot: signal is "TRUE";
begin
    zynq_bd_i: entity work.zynq_bd
        port map(
            clk_250m => clk_250m,
            clk_250m_resetn(0) => clk_250m_sresetn,
            acc_axil_awready => axil_awready,
            acc_axil_awvalid => axil_awvalid,
            acc_axil_awprot => axil_awprot,
            acc_axil_awaddr => axil_awaddr,
            acc_axil_wready => axil_wready,
            acc_axil_wvalid =>axil_wvalid,
            acc_axil_wstrb => axil_wstrb,
            acc_axil_wdata => axil_wdata,
            acc_axil_bready => axil_bready,
            acc_axil_bvalid => axil_bvalid,
            acc_axil_bresp => axil_bresp,
            acc_axil_arready => axil_arready,
            acc_axil_arvalid => axil_arvalid,
            acc_axil_arprot => axil_arprot,
            acc_axil_araddr => axil_araddr,
            acc_axil_rready => axil_rready,
            acc_axil_rvalid => axil_rvalid,
            acc_axil_rresp => axil_rresp,
            acc_axil_rdata => axil_rdata
        );
    
    clk_250m_sreset <= not clk_250m_sresetn;

    emulator_top_i: entity work.emulator_top
        generic map(
            X_SIZE => x_size,
            Y_SIZE => y_size,
            AXI_DATA_BITS => AXI_DATA_BITS
        )
        port map(
            clk_in => clk_250m,
            sreset_in => clk_250m_sreset,
            axil_awready => axil_awready,
            axil_awvalid => axil_awvalid,
            axil_awprot => axil_awprot,
            axil_awaddr => axil_awaddr,
            axil_wready => axil_wready,
            axil_wvalid => axil_wvalid,
            axil_wstrb => axil_wstrb,
            axil_wdata => axil_wdata,
            axil_bready => axil_bready,
            axil_bvalid => axil_bvalid,
            axil_bresp => axil_bresp,
            axil_arready => axil_arready,
            axil_arvalid => axil_arvalid,
            axil_arprot => axil_arprot,
            axil_araddr => axil_araddr,
            axil_rready => axil_rready,
            axil_rvalid => axil_rvalid,
            axil_rresp => axil_rresp,
            axil_rdata => axil_rdata
        );
end;

