library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

-- write_hw_platform -fixed -include_bit -force -file /home/topi/work/fpga-kiihdytys/vivado_kiihdytys/accelerator_top.xsa

entity axil_slave is
    generic(
        rd_dwords: natural;
        rd_offset: natural;
        wr_dwords: natural;
        wr_offset: natural;
        AXI_DATA_BITS: positive
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
        axil_rdata: out std_logic_vector(AXI_DATA_BITS-1 downto 0);

        rd_data_in: in std_logic_vector(rd_dwords*32-1 downto 0);
        wr_data_out: out std_logic_vector(wr_dwords*32-1 downto 0);

        run_reg_out: out std_logic_vector(31 downto 0);
        run_reg_0_pulse_out: out std_logic
    );
end;

architecture rtl of axil_slave is
    constant rd_bits: natural := rd_dwords*32;
    constant wr_bits: natural := wr_dwords*32;

    constant IGNORE_ADD_LSBS: natural := integer(log2(real(AXI_DATA_BITS/8)));

    constant run_add: natural := 0;
    
    constant rd_start: natural := rd_offset;
    constant rd_end: natural := rd_start + rd_dwords - 1;

    constant wr_start: natural := wr_offset;
    constant wr_end: natural := wr_start + wr_dwords - 1;

    signal rd_data: std_logic_vector(rd_bits-1 downto 0);
    signal wr_data: std_logic_vector(wr_bits-1 downto 0);

    signal ar_d1_ready: std_logic;
    signal ar_d1_valid: std_logic;
    signal ar_d1_prot: std_logic_vector(2 downto 0);
    signal ar_d1_addr: std_logic_vector(15 downto 0);
    signal ar_d1_data: std_logic_vector(AXI_DATA_BITS-1 downto 0);

    signal axil_wr: std_logic;

    signal b_cnt: unsigned(7 downto 0);

    signal run_reg: std_logic_vector(31 downto 0);
    signal run_reg_0_pulse: std_logic;

    signal free_counter: unsigned(31 downto 0);
begin
    process(clk_in)
    begin
        if rising_edge(clk_in) then
            free_counter <= free_counter + 1;
        end if;
    end process;

    process(all)
    begin
        rd_data <= rd_data_in;
    end process;

    process(all)
    begin
        axil_arready <= '0';
        if ar_d1_valid = '0' or ar_d1_ready = '1' then
            axil_arready <= '1';
        end if;

        if sreset_in = '1' then
            axil_arready <= '0';
        end if;
    end process;

    process(clk_in)
        variable addr: unsigned(15 downto 0);
        variable pos: unsigned(15 downto 0);
    begin
        if rising_edge(clk_in) then
            if axil_arready = '1' then
                addr := shift_right(unsigned(axil_araddr), IGNORE_ADD_LSBS);
                ar_d1_valid <= axil_arvalid;
                ar_d1_prot <= axil_arprot;
                ar_d1_addr <= axil_araddr;
                for i in 0 to AXI_DATA_BITS/32-1 loop
                    ar_d1_data(i*32-31 downto i*32) <= X"DEAD_BEEF";
                end loop;
                if addr >= rd_start and addr <= rd_end then
                    pos := addr - rd_start;
                    ar_d1_data <= rd_data(to_integer(pos)*AXI_DATA_BITS+AXI_DATA_BITS-1 downto to_integer(pos)*AXI_DATA_BITS);
                end if;
                if addr = 0 then
                    ar_d1_data(31 downto 0) <= std_logic_vector(free_counter);
                end if;
            end if;
        end if;
    end process;

    process(all)
    begin
        ar_d1_ready <= axil_rready;
        axil_rvalid <= ar_d1_valid;
        axil_rresp <= "00";
        axil_rdata <= ar_d1_data;
    end process;

    process(all)
    begin
        axil_awready <= '0';
        axil_wready <= '0';
        axil_wr <= '0';

        if axil_awvalid = '1' and axil_wvalid = '1' then
            axil_awready <= '1';
            axil_wready <= '1';
            axil_wr <= '1';
        end if;

        if sreset_in = '1' then
            axil_awready <= '0';
            axil_wready <= '0';
            axil_wr <= '0';
        end if;
    end process;

    process(clk_in)
        variable v_b_cnt: unsigned(7 downto 0);
    begin
        if rising_edge(clk_in) then
            v_b_cnt := b_cnt;

            if axil_wr = '1' then
                v_b_cnt := v_b_cnt + 1;
            end if;

            if axil_bready = '1' and axil_bvalid = '1' then
                v_b_cnt := v_b_cnt - 1;
            end if;

            b_cnt <= v_b_cnt;

            if sreset_in = '1' then
                b_cnt <= (others => '0');
            end if;
        end if;
    end process;

    process(all)
    begin
        axil_bvalid <= '0';
        axil_bresp <= "00";

        if b_cnt > 0 then
            axil_bvalid <= '1';
        end if;

        if sreset_in = '1' then
            axil_bvalid <= '0';
        end if;
    end process;

    process(clk_in)
        variable addr: unsigned(15 downto 0);
        variable pos: unsigned(15 downto 0);
    begin
        if rising_edge(clk_in) then
            run_reg_0_pulse <= '0';

            if axil_wr = '1' then
                addr := shift_right(unsigned(axil_awaddr), IGNORE_ADD_LSBS);

                if addr = run_add then
                    run_reg <= axil_wdata(31 downto 0);
                    if axil_wdata(0) = '1' then
                        run_reg_0_pulse <= '1';
                    end if;
                end if;

                if addr >= wr_start and addr <= wr_end then
                    pos := addr - wr_start;
                    -- wr_data(to_integer(pos)*AXI_DATA_BITS+AXI_DATA_BITS-1 downto to_integer(pos)*AXI_DATA_BITS) <= axil_wdata;
                    for i in axil_wstrb'range loop
                        if axil_wstrb(i) = '1' then
                            wr_data(to_integer(pos)*AXI_DATA_BITS+i*8+7 downto to_integer(pos)*AXI_DATA_BITS+i*8) <= axil_wdata(i*8+7 downto i*8);
                        end if;
                    end loop;
                end if;
            end if;
        end if;
    end process;

    process(all)
    begin
        wr_data_out <= wr_data;
        run_reg_out <= run_reg;
        run_reg_0_pulse_out <= run_reg_0_pulse;
    end process;
end;
