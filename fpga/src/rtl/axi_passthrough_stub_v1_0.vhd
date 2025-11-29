library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity axi_passthrough_stub_v1_0 is
  generic (
    C_S_AXI_DATA_WIDTH : integer := 32;
    C_S_AXI_ADDR_WIDTH : integer := 32
  );
  port (
    -- AXI slave interface (seen by Vivado BD / PS master)
    S_AXI_ACLK    : in  std_logic;
    S_AXI_ARESETN : in  std_logic;
    S_AXI_AWADDR  : in  std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
    S_AXI_AWPROT  : in  std_logic_vector(2 downto 0);
    S_AXI_AWVALID : in  std_logic;
    S_AXI_AWREADY : out std_logic;
    S_AXI_WDATA   : in  std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
    S_AXI_WSTRB   : in  std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
    S_AXI_WVALID  : in  std_logic;
    S_AXI_WREADY  : out std_logic;
    S_AXI_BRESP   : out std_logic_vector(1 downto 0);
    S_AXI_BVALID  : out std_logic;
    S_AXI_BREADY  : in  std_logic;
    S_AXI_ARADDR  : in  std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
    S_AXI_ARPROT  : in  std_logic_vector(2 downto 0);
    S_AXI_ARVALID : in  std_logic;
    S_AXI_ARREADY : out std_logic;
    S_AXI_RDATA   : out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
    S_AXI_RRESP   : out std_logic_vector(1 downto 0);
    S_AXI_RVALID  : out std_logic;
    S_AXI_RREADY  : in  std_logic;

    -- External (real) AXI slave ports
    EXT_ACLK: in std_logic;
    EXT_ARESETN: in std_logic;
    EXT_AWADDR  : out std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
    EXT_AWPROT  : out std_logic_vector(2 downto 0);
    EXT_AWVALID : out std_logic;
    EXT_AWREADY : in  std_logic;
    EXT_WDATA   : out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
    EXT_WSTRB   : out std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
    EXT_WVALID  : out std_logic;
    EXT_WREADY  : in  std_logic;
    EXT_BRESP   : in  std_logic_vector(1 downto 0);
    EXT_BVALID  : in  std_logic;
    EXT_BREADY  : out std_logic;
    EXT_ARADDR  : out std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
    EXT_ARPROT  : out std_logic_vector(2 downto 0);
    EXT_ARVALID : out std_logic;
    EXT_ARREADY : in  std_logic;
    EXT_RDATA   : in  std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
    EXT_RRESP   : in  std_logic_vector(1 downto 0);
    EXT_RVALID  : in  std_logic;
    EXT_RREADY  : out std_logic
  );
end entity;

architecture rtl of axi_passthrough_stub_v1_0 is
begin
  -- Write address channel
  EXT_AWADDR  <= S_AXI_AWADDR;
  EXT_AWPROT  <= S_AXI_AWPROT;
  EXT_AWVALID <= S_AXI_AWVALID;
  S_AXI_AWREADY <= EXT_AWREADY;

  -- Write data channel
  EXT_WDATA   <= S_AXI_WDATA;
  EXT_WSTRB   <= S_AXI_WSTRB;
  EXT_WVALID  <= S_AXI_WVALID;
  S_AXI_WREADY <= EXT_WREADY;

  -- Write response channel
  S_AXI_BRESP  <= EXT_BRESP;
  S_AXI_BVALID <= EXT_BVALID;
  EXT_BREADY   <= S_AXI_BREADY;

  -- Read address channel
  EXT_ARADDR  <= S_AXI_ARADDR;
  EXT_ARPROT  <= S_AXI_ARPROT;
  EXT_ARVALID <= S_AXI_ARVALID;
  S_AXI_ARREADY <= EXT_ARREADY;

  -- Read data channel
  S_AXI_RDATA  <= EXT_RDATA;
  S_AXI_RRESP  <= EXT_RRESP;
  S_AXI_RVALID <= EXT_RVALID;
  EXT_RREADY   <= S_AXI_RREADY;
end architecture;
