/*
 * soc_base.v
 *
 * vim: ts=4 sw=4
 *
 * Minimal common base for the E1 project SoC
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: CERN-OHL-S-2.0
 */

`default_nettype none

module soc_base #(
	parameter integer WB_N = 1,
	parameter integer E1_N = 1,
	parameter         E1_UNIT_HAS_RX = 1'b1,
	parameter         E1_UNIT_HAS_TX = 1'b1,
	parameter integer E1_LIU = 0
)(
	// E1 pads
		// Raw PHY
	input  wire [E1_N-1:0] e1_rx_hi_p,
	input  wire [E1_N-1:0] e1_rx_hi_n,
	input  wire [E1_N-1:0] e1_rx_lo_p,
	input  wire [E1_N-1:0] e1_rx_lo_n,

	output wire [E1_N-1:0] e1_tx_hi,
	output wire [E1_N-1:0] e1_tx_lo,

		// LIU
	input  wire [E1_N-1:0] e1_rx_data,
	input  wire [E1_N-1:0] e1_rx_clk,

	output wire [E1_N-1:0] e1_tx_data,
	output wire [E1_N-1:0] e1_tx_clk,

	// USB
	inout  wire usb_dp,
	inout  wire usb_dn,
	output wire usb_pu,

	// Flash SPI (raw)
	input  wire flash_mosi_i,
	output wire flash_mosi_o,
	output wire flash_mosi_oe,

	input  wire flash_miso_i,
	output wire flash_miso_o,
	output wire flash_miso_oe,

	input  wire flash_clk_i,
	output wire flash_clk_o,
	output wire flash_clk_oe,

	output wire flash_csn_o,

	// Debug UART
	input  wire dbg_rx,
	output wire dbg_tx,

	// RGB LEDs
	output wire [2:0] rgb,

	// External Master Wishbone bus (CPU -> Peripheral)
	output wire          [15:0] wb_m_addr,
	input  wire [(WB_N*32)-1:0] wb_m_rdata,
	output wire          [31:0] wb_m_wdata,
	output wire          [ 3:0] wb_m_wmsk,
	output wire                 wb_m_we,
	output wire [ WB_N    -1:0] wb_m_cyc,
	input  wire [ WB_N    -1:0] wb_m_ack,

	// Ticks
	output wire [E1_N-1:0] tick_e1_rx,
	output wire [E1_N-1:0] tick_e1_tx,
	output wire            tick_usb_sof,

	// Clock / Reset
	input  wire clk_sys,
	input  wire rst_sys,
	input  wire clk_48m,
	input  wire rst_48m
);

	genvar i;

	localparam integer WB_LN = 8;
	localparam integer WB_TN = WB_N + WB_LN;
	localparam integer WB_DW = 32;
	localparam integer WB_MW = WB_DW / 8;
	localparam integer WB_AW = 16;
	localparam integer WB_AI =  2;


	// Signals
	// -------

	// Picorv32 native bus
	wire        pb_valid;
	wire        pb_instr;
	wire        pb_ready;
	wire [31:0] pb_addr;
	wire [31:0] pb_rdata;
	wire [31:0] pb_wdata;
	wire [ 3:0] pb_wstrb;

	// SoC RAM
		// BRAM
	wire [ 7:0] bram_addr;
	wire [31:0] bram_rdata;
	wire [31:0] bram_wdata;
	wire [ 3:0] bram_wmsk;
	wire        bram_we;

		// SPRAM
	wire [14:0] spram_addr;
	wire [31:0] spram_rdata;
	wire [31:0] spram_wdata;
	wire [ 3:0] spram_wmsk;
	wire        spram_we;

	// Peripheral wishbone
	wire [WB_AW-1:0] wb_addr;
	wire [WB_DW-1:0] wb_rdata [0:WB_LN-1];
	wire [WB_DW-1:0] wb_wdata;
	wire [WB_MW-1:0] wb_wmsk;
	wire             wb_we;
	wire [WB_TN-1:0] wb_cyc;
	wire [WB_TN-1:0] wb_ack;

	wire [(WB_DW*WB_TN)-1:0] wb_rdata_flat;

	// USB
		// Wishbone ( @ 48 MHz )
	wire [11:0] ub_addr;
	wire [15:0] ub_rdata;
	wire [15:0] ub_wdata;
	wire        ub_we;
	wire        ub_cyc;
	wire        ub_ack;

		// EP interface
	wire [ 8:0] ep_tx_addr_0;
	wire [31:0] ep_tx_data_0;
	wire        ep_tx_we_0;

	wire [ 8:0] ep_rx_addr_0;
	wire [31:0] ep_rx_data_1;
	wire        ep_rx_re_0;

		// SoF
	wire usb_sof;

	// Wishbone bus E1 to IO buffers
	wire [13:0] wb_e1_addr;
	wire [31:0] wb_e1_rdata;
	wire [31:0] wb_e1_wdata;
	wire [ 3:0] wb_e1_wmsk;
	wire        wb_e1_we;
	wire        wb_e1_cyc;
	wire        wb_e1_ack;

	// E1 buffer interface
	wire [(E1_N*8)-1:0] e1_buf_rx_data;
	wire [(E1_N*5)-1:0] e1_buf_rx_ts;
	wire [(E1_N*4)-1:0] e1_buf_rx_frame;
	wire [(E1_N*7)-1:0] e1_buf_rx_mf;
	wire [ E1_N   -1:0] e1_buf_rx_we;
	wire [ E1_N   -1:0] e1_buf_rx_rdy;
	wire [(E1_N*8)-1:0] e1_buf_tx_data;
	wire [(E1_N*5)-1:0] e1_buf_tx_ts;
	wire [(E1_N*4)-1:0] e1_buf_tx_frame;
	wire [(E1_N*7)-1:0] e1_buf_tx_mf;
	wire [ E1_N   -1:0] e1_buf_tx_re;
	wire [ E1_N   -1:0] e1_buf_tx_rdy;


	// SoC core
	// --------

	// Local CPU reset
	reg pb_rst_n;

	always @(posedge clk_sys or posedge rst_sys)
		if (rst_sys)
			pb_rst_n <= 1'b0;
		else
			pb_rst_n <= 1'b1;

	// CPU
	picorv32 #(
		.PROGADDR_RESET(32'h 0000_0000),
		.STACKADDR(32'h 0000_0400),
		.BARREL_SHIFTER(0),
`ifdef BOARD_E1_TRACER
		.TWO_CYCLE_COMPARE(0),
		.TWO_CYCLE_ALU(0),
`else
		.TWO_CYCLE_COMPARE(0),
		.TWO_CYCLE_ALU(1),
`endif
		.COMPRESSED_ISA(0),
		.ENABLE_COUNTERS(0),
		.ENABLE_MUL(0),
		.ENABLE_DIV(0),
		.ENABLE_IRQ(0),
		.ENABLE_IRQ_QREGS(0),
		.CATCH_MISALIGN(0),
		.CATCH_ILLINSN(0)
	) cpu_I (
		.clk       (clk_sys),
		.resetn    (pb_rst_n),
		.mem_valid (pb_valid),
		.mem_instr (pb_instr),
		.mem_ready (pb_ready),
		.mem_addr  (pb_addr),
		.mem_wdata (pb_wdata),
		.mem_wstrb (pb_wstrb),
		.mem_rdata (pb_rdata)
	);

	// CPU bridge
	soc_picorv32_bridge #(
		.WB_N  (WB_TN),
		.WB_DW (32),
		.WB_AW (16),
		.WB_AI ( 2),
		.WB_REG( 0)
	) bridge_I (
		.pb_addr    (pb_addr),
		.pb_rdata   (pb_rdata),
		.pb_wdata   (pb_wdata),
		.pb_wstrb   (pb_wstrb),
		.pb_valid   (pb_valid),
		.pb_ready   (pb_ready),
		.bram_addr  (bram_addr),
		.bram_rdata (bram_rdata),
		.bram_wdata (bram_wdata),
		.bram_wmsk  (bram_wmsk),
		.bram_we    (bram_we),
		.spram_addr (spram_addr),
		.spram_rdata(spram_rdata),
		.spram_wdata(spram_wdata),
		.spram_wmsk (spram_wmsk),
		.spram_we   (spram_we),
		.wb_addr    (wb_addr),
		.wb_rdata   (wb_rdata_flat),
		.wb_wdata   (wb_wdata),
		.wb_wmsk    (wb_wmsk),
		.wb_we      (wb_we),
		.wb_cyc     (wb_cyc),
		.wb_ack     (wb_ack),
		.clk        (clk_sys),
		.rst        (rst_sys)
	);

	for (i=0; i<WB_LN; i=i+1)
		assign wb_rdata_flat[i*WB_DW+:WB_DW] = wb_rdata[i];

	// Boot memory - 1k
	soc_bram #(
		.AW(8),
		.INIT_FILE("boot.hex"),
	) bram_I (
		.addr (bram_addr),
		.rdata(bram_rdata),
		.wdata(bram_wdata),
		.wmsk (bram_wmsk),
		.we   (bram_we),
		.clk  (clk_sys)
	);

	// Main SoC memory - 64k
	soc_spram #(
		.AW(14)
	) spram_I (
		.addr (spram_addr[13:0]),
		.rdata(spram_rdata),
		.wdata(spram_wdata),
		.wmsk (spram_wmsk),
		.we   (spram_we),
		.clk  (clk_sys)
	);

	// Peripheral wishbone export
	assign wb_m_addr  = wb_addr;
	assign wb_m_wdata = wb_wdata;
	assign wb_m_wmsk  = wb_wmsk;
	assign wb_m_we    = wb_we;
	assign wb_m_cyc   = wb_cyc[WB_TN-1:WB_LN];

	assign wb_rdata_flat[(WB_TN*WB_DW)-1:(WB_LN*WB_DW)] = wb_m_rdata;
	assign wb_ack[WB_TN-1:WB_LN] = wb_m_ack;


	// SPI [0]
	// ---

	ice40_spi_wb #(
		.N_CS(1),
		.WITH_IOB(0),
		.UNIT(0)
	) spi_I (
		.sio_mosi_i (flash_mosi_i),
		.sio_mosi_o (flash_mosi_o),
		.sio_mosi_oe(flash_mosi_oe),
		.sio_miso_i (flash_miso_i),
		.sio_miso_o (flash_miso_o),
		.sio_miso_oe(flash_miso_oe),
		.sio_clk_i  (flash_clk_i),
		.sio_clk_o  (flash_clk_o),
		.sio_clk_oe (flash_clk_oe),
		.sio_csn_o  (flash_csn_o),
		.sio_csn_oe (),
		.wb_addr    (wb_addr[3:0]),
		.wb_rdata   (wb_rdata[0]),
		.wb_wdata   (wb_wdata),
		.wb_we      (wb_we),
		.wb_cyc     (wb_cyc[0]),
		.wb_ack     (wb_ack[0]),
		.clk        (clk_sys),
		.rst        (rst_sys)
	);


	// Debug UART [1]
	// ----------

	uart_wb #(
		.DIV_WIDTH(12),
		.DW(WB_DW)
	) uart_I (
		.uart_tx  (dbg_tx),
		.uart_rx  (dbg_rx),
		.wb_addr  (wb_addr[1:0]),
		.wb_rdata (wb_rdata[1]),
		.wb_wdata (wb_wdata),
		.wb_we    (wb_we),
		.wb_cyc   (wb_cyc[1]),
		.wb_ack   (wb_ack[1]),
		.clk      (clk_sys),
		.rst      (rst_sys)
	);


	// RGB LEDs [2]
	// --------

	ice40_rgb_wb #(
		.CURRENT_MODE("0b1"),
		.RGB0_CURRENT("0b000001"),
		.RGB1_CURRENT("0b000001"),
		.RGB2_CURRENT("0b000001")
	) rgb_I (
		.pad_rgb    (rgb),
		.wb_addr    (wb_addr[4:0]),
		.wb_rdata   (wb_rdata[2]),
		.wb_wdata   (wb_wdata),
		.wb_we      (wb_we),
		.wb_cyc     (wb_cyc[2]),
		.wb_ack     (wb_ack[2]),
		.clk        (clk_sys),
		.rst        (rst_sys)
	);


	// USB [3]
	// ---

	// Core instance ( @ 48 MHz )
	usb #(
		.EPDW(32)
	) usb_I (
		.pad_dp      (usb_dp),
		.pad_dn      (usb_dn),
		.pad_pu      (usb_pu),
		.ep_tx_addr_0(ep_tx_addr_0),
		.ep_tx_data_0(ep_tx_data_0),
		.ep_tx_we_0  (ep_tx_we_0),
		.ep_rx_addr_0(ep_rx_addr_0),
		.ep_rx_data_1(ep_rx_data_1),
		.ep_rx_re_0  (ep_rx_re_0),
		.ep_clk      (clk_sys),
		.wb_addr     (ub_addr),
		.wb_rdata    (ub_rdata),
		.wb_wdata    (ub_wdata),
		.wb_we       (ub_we),
		.wb_cyc      (ub_cyc),
		.wb_ack      (ub_ack),
		.sof         (usb_sof),
		.clk         (clk_48m),
		.rst         (rst_48m)
	);

    // Cross clock bridge
	xclk_wb #(
		.DW(16),
		.AW(12)
	) wb_48m_xclk_I (
		.s_addr (wb_addr[11:0]),
		.s_rdata(wb_rdata[3][15:0]),
		.s_wdata(wb_wdata[15:0]),
		.s_we   (wb_we),
		.s_cyc  (wb_cyc[3]),
		.s_ack  (wb_ack[3]),
		.s_clk  (clk_sys),
		.m_addr (ub_addr),
		.m_rdata(ub_rdata),
		.m_wdata(ub_wdata),
		.m_we   (ub_we),
		.m_cyc  (ub_cyc),
		.m_ack  (ub_ack),
		.m_clk  (clk_48m),
		.rst    (rst_sys)
	);

	assign wb_rdata[3][31:16] = 16'h0000;

	// Cross clock SoF
	xclk_strobe sof_xclk_I (
		.in_stb (usb_sof),
		.in_clk (clk_48m),
		.out_stb(tick_usb_sof),
		.out_clk(clk_sys),
		.rst    (rst_sys)
	);


	// IO buffers & DMA
	// ----------------
	// [4] USB EP buffer
	// [5] E1 SPRAM buffer
	// [6] DMA

	soc_iobuf iobuf_I (
		.wb_cpu_addr (wb_addr),
		.wb_cpu_rdata(wb_rdata[4]),
		.wb_cpu_wdata(wb_wdata),
		.wb_cpu_wmsk (wb_wmsk),
		.wb_cpu_we   (wb_we),
		.wb_cpu_cyc  (wb_cyc[6:4]),
		.wb_cpu_ack  (wb_ack[6:4]),
		.wb_e1_addr  (wb_e1_addr),
		.wb_e1_rdata (wb_e1_rdata),
		.wb_e1_wdata (wb_e1_wdata),
		.wb_e1_wmsk  (wb_e1_wmsk),
		.wb_e1_we    (wb_e1_we),
		.wb_e1_cyc   (wb_e1_cyc),
		.wb_e1_ack   (wb_e1_ack),
		.ep_tx_addr_0(ep_tx_addr_0),
		.ep_tx_data_0(ep_tx_data_0),
		.ep_tx_we_0  (ep_tx_we_0),
		.ep_rx_addr_0(ep_rx_addr_0),
		.ep_rx_data_1(ep_rx_data_1),
		.ep_rx_re_0  (ep_rx_re_0),
		.clk         (clk_sys),
		.rst         (rst_sys)
	);

	assign wb_rdata[5] = 32'h00000000;
	assign wb_rdata[6] = 32'h00000000;


	// E1 [7]
	// --

	// E1 wishbone module
	e1_wb #(
		.N(E1_N),
		.UNIT_HAS_RX(E1_UNIT_HAS_RX),
		.UNIT_HAS_TX(E1_UNIT_HAS_TX),
		.LIU(E1_LIU),
		.MFW(7)
	) e1_I (
		.pad_rx_hi_p (e1_rx_hi_p),
		.pad_rx_hi_n (e1_rx_hi_n),
		.pad_rx_lo_p (e1_rx_lo_p),
		.pad_rx_lo_n (e1_rx_lo_n),
		.pad_tx_hi   (e1_tx_hi),
		.pad_tx_lo   (e1_tx_lo),
		.pad_rx_data (e1_rx_data),
		.pad_rx_clk  (e1_rx_clk),
		.pad_tx_data (e1_tx_data),
		.pad_tx_clk  (e1_tx_clk),
		.buf_rx_data (e1_buf_rx_data),
		.buf_rx_ts   (e1_buf_rx_ts),
		.buf_rx_frame(e1_buf_rx_frame),
		.buf_rx_mf   (e1_buf_rx_mf),
		.buf_rx_we   (e1_buf_rx_we),
		.buf_rx_rdy  (e1_buf_rx_rdy),
		.buf_tx_data (e1_buf_tx_data),
		.buf_tx_ts   (e1_buf_tx_ts),
		.buf_tx_frame(e1_buf_tx_frame),
		.buf_tx_mf   (e1_buf_tx_mf),
		.buf_tx_re   (e1_buf_tx_re),
		.buf_tx_rdy  (e1_buf_tx_rdy),
		.wb_addr     (wb_addr[7:0]),
		.wb_rdata    (wb_rdata[7][15:0]),
		.wb_wdata    (wb_wdata[15:0]),
		.wb_we       (wb_we),
		.wb_cyc      (wb_cyc[7]),
		.wb_ack      (wb_ack[7]),
		.irq         (),
		.tick_rx     (tick_e1_rx),
		.tick_tx     (tick_e1_tx),
		.clk         (clk_sys),
		.rst         (rst_sys)
	);

	assign wb_rdata[7][31:16] = 16'h0000;

	// E1 buffer interface to Wishbone
	e1_buf_if_wb #(
		.N(E1_N),
		.UNIT_HAS_RX(E1_UNIT_HAS_RX),
		.UNIT_HAS_TX(E1_UNIT_HAS_TX),
		.MFW(7),
		.DW(32)
	) e1_buf_I (
		.wb_addr     (wb_e1_addr),
		.wb_rdata    (wb_e1_rdata),
		.wb_wdata    (wb_e1_wdata),
		.wb_wmsk     (wb_e1_wmsk),
		.wb_we       (wb_e1_we),
		.wb_cyc      (wb_e1_cyc),
		.wb_ack      (wb_e1_ack),
		.buf_rx_data (e1_buf_rx_data),
		.buf_rx_ts   (e1_buf_rx_ts),
		.buf_rx_frame(e1_buf_rx_frame),
		.buf_rx_mf   (e1_buf_rx_mf),
		.buf_rx_we   (e1_buf_rx_we),
		.buf_rx_rdy  (e1_buf_rx_rdy),
		.buf_tx_data (e1_buf_tx_data),
		.buf_tx_ts   (e1_buf_tx_ts),
		.buf_tx_frame(e1_buf_tx_frame),
		.buf_tx_mf   (e1_buf_tx_mf),
		.buf_tx_re   (e1_buf_tx_re),
		.buf_tx_rdy  (e1_buf_tx_rdy),
		.clk         (clk_sys),
		.rst         (rst_sys)
	);

endmodule // soc_base
