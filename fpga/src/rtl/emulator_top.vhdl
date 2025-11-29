library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.vhdl_linkruncca_pkg.all;

entity emulator_top is
    generic(
        X_SIZE: positive := x_size;
        Y_SIZE: positive := 1024;
        AXI_DATA_BITS: positive := 64
    );
    port(
        clk_in: in std_logic;
        sreset_in: in std_logic;

        axil_awready: out std_logic;
        axil_awvalid: in std_logic;
        axil_awprot: in std_logic_vector(2 downto 0);
        axil_awaddr: in std_logic_vector(15 downto 0);

        axil_wready: out std_logic;
        axil_wvalid: in std_logic;
        axil_wstrb: in std_logic_vector(AXI_DATA_BITS/8-1 downto 0);
        axil_wdata: in std_logic_vector(AXI_DATA_BITS-1 downto 0);

        axil_bready: in std_logic;
        axil_bvalid: out std_logic;
        axil_bresp: out std_logic_vector(1 downto 0);

        axil_arready: out std_logic;
        axil_arvalid: in std_logic;
        axil_arprot: in std_logic_vector(2 downto 0);
        axil_araddr: in std_logic_vector(15 downto 0);

        axil_rready: in std_logic;
        axil_rvalid: out std_logic;
        axil_rresp: out std_logic_vector(1 downto 0);
        axil_rdata: out std_logic_vector(AXI_DATA_BITS-1 downto 0)
    );
end;

architecture rtl of emulator_top is
    component BUFGCE is
        generic(
            CE_TYPE: string := "SYNC"
        );
        port(
            CE: in std_logic;
            I: in std_logic;
            O: out std_logic
        );
    end component;

    -- signal tmp_pix_in: linkruncca_collect_t;

    type feed_t is record
        rst: std_logic;
        datavalid: std_logic;
        pix_in: linkruncca_collect_t;
    end record;

    function to_slv(a: feed_t) return std_logic_vector is
        constant tmp: std_logic_vector := to_slv(a.pix_in);
        constant bits: natural := 1 + 1 + tmp'length;
        variable v: std_logic_vector(bits-1 downto 0);
        variable p: natural;
    begin
        p := 0;
        to_bits(a.rst, p, v);
        to_bits(a.datavalid, p, v);
        to_bits(a.pix_in, p, v);
        return v;
    end;

    function from_slv(v: std_logic_vector) return feed_t is
        variable r: feed_t;
        variable p: natural;
    begin
        p := 0;
        from_bits(r.rst, p, v);
        from_bits(r.datavalid, p, v);
        from_bits(r.pix_in, p, v);
        return r;
    end;

    function feed_bits return natural is
        variable feed: feed_t;
        constant tmp: std_logic_vector := to_slv(feed);
    begin
        return tmp'length;
    end;

    constant feed_dwords: natural := (feed_bits + 31) / 32;

    type res_t is record
        res_valid_out: std_logic;
        res_data_out: linkruncca_feature_t;
    end record;

    function to_slv(a: res_t) return std_logic_vector is
        constant tmp: std_logic_vector := to_slv(a.res_data_out);
        constant bits: natural := 1 + tmp'length;
        variable v: std_logic_vector(bits-1 downto 0);
        variable p: natural;
    begin
        p := 0;
        to_bits(a.res_valid_out, p, v);
        to_bits(a.res_data_out, p, v);
        return v;
    end;

    function from_slv(v: std_logic_vector) return res_t is
        variable r: res_t;
        variable p: natural;
    begin
        p := 0;
        from_bits(r.res_valid_out, p, v);
        from_bits(r.res_data_out, p, v);
        return r;
    end;

    function res_bits return natural is
        variable res: res_t;
        constant tmp: std_logic_vector := to_slv(res);
    begin
        return tmp'length;
    end;

    constant res_dwords: natural := (res_bits + 31) / 32;

    signal feed_axil: std_logic_vector(feed_dwords*32-1 downto 0);
    signal feed_slv: std_logic_vector(feed_bits-1 downto 0);
    signal feed: feed_t;

    signal res: res_t;
    signal res_slv: std_logic_vector(res_bits-1 downto 0);
    signal res_axil: std_logic_vector(res_dwords*32-1 downto 0);

    constant run_add: natural := 0;
    signal run_reg: std_logic_vector(31 downto 0);
    signal run_reg_0_pulse: std_logic;

    constant feed_start: natural := 16;
    constant feed_end: natural := feed_start + feed_dwords;

    constant res_start: natural := 16;
    constant res_end: natural := res_start + res_dwords;

    signal dut_clk_req: std_logic;

    signal dut_clk: std_logic;
begin
    process(all)
    begin
        feed_slv <= feed_axil(feed_bits-1 downto 0);
        feed <= from_slv(feed_slv);
        
        res_slv <= to_slv(res);
        res_axil <= (others => '0');
        res_axil(res_bits-1 downto 0) <= res_slv;
    end process;

    axil_slave_i: entity work.axil_slave
        generic map(
            rd_dwords => res_dwords,
            rd_offset => 16,
            wr_dwords => feed_dwords,
            wr_offset => 16,
            AXI_DATA_BITS => AXI_DATA_BITS
        )
        port map(
            clk_in => clk_in,
            sreset_in => sreset_in,
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
            axil_rdata => axil_rdata,
            rd_data_in => res_axil,
            wr_data_out => feed_axil,
            run_reg_out => run_reg,
            run_reg_0_pulse_out => run_reg_0_pulse
        );

    dut_clk_req <= run_reg_0_pulse;

    pulsed_clock_buf_i: BUFGCE
        generic map (
            CE_TYPE => "SYNC"
        )
        port map (
            CE => dut_clk_req,
            I => clk_in,
            O => dut_clk
        );

    dut: entity work.vhdl_linkruncca
        generic map(
            imwidth => X_SIZE,
            imheight => Y_SIZE
        )
        port map(
            clk => dut_clk,
            rst => feed.rst,
            datavalid => feed.datavalid,
            pix_in => feed.pix_in,
            res_valid_out => res.res_valid_out,
            res_data_out => res.res_data_out
        );
end;