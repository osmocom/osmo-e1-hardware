/*
 * top.v
 *
 * vim: ts=4 sw=4
 *
 * Top-level for the e1-tracer boards.
 *
 * Note that some things here are only to maintain bitstream compatibility
 * with the icepick based proto setup.
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: CERN-OHL-S-2.0
 */

`default_nettype none

module top (
	// LIU data
	input  wire e1A_rx_data,
	input  wire e1A_rx_clk,
	input  wire e1B_rx_data,
	input  wire e1B_rx_clk,

	// LIU control
	inout  wire       liu_mosi,
	inout  wire       liu_miso,
	inout  wire       liu_clk,
	inout  wire [1:0] liu_cs_n,

	// USB
	inout  wire usb_dp,
	inout  wire usb_dn,
	output wire usb_pu,

	// Flash
	inout  wire flash_mosi,
	inout  wire flash_miso,
	inout  wire flash_clk,
	inout  wire flash_cs_n,

	// VIO PDM
	output wire vio_pdm,

	// Button
	input  wire btn,

	// Clock (12 MHz)
	input  wire clk_in,

	// Debug UART
	output wire dbg_tx,

	// RGB LEDs
	output wire [2:0] rgb
);

	localparam integer WB_N = 2;

	genvar i;


	// Signals
	// -------

	// Flash SPI internal signals
	wire flash_mosi_i,  flash_miso_i,  flash_clk_i;
	wire flash_mosi_o,  flash_miso_o,  flash_clk_o;
	wire flash_mosi_oe, flash_miso_oe, flash_clk_oe;
	wire flash_csn_o;

	// Peripheral wishbone
	wire     [15:0] wb_addr;
	wire     [31:0] wb_rdata [0:WB_N-1];
	wire     [31:0] wb_wdata;
	wire     [ 3:0] wb_wmsk;
	wire            wb_we;
	wire [WB_N-1:0] wb_cyc;
	wire [WB_N-1:0] wb_ack;

	wire [(WB_N*32)-1:0] wb_rdata_flat;

	// Ticks
	wire [1:0] tick_e1_rx;
	wire       tick_usb_sof;

	// Clocks / Reset
	wire rst_req;

	wire clk_sys;
	wire rst_sys;
	wire clk_48m;
	wire rst_48m;


	// SoC base
	// --------

	// Instance
	soc_base #(
		.WB_N(WB_N),
		.E1_N(2),
		.E1_UNIT_HAS_RX(2'b11),
		.E1_UNIT_HAS_TX(2'b00),
		.E1_LIU(1)
	) soc_I (
		.e1_rx_hi_p   (),
		.e1_rx_hi_n   (),
  		.e1_rx_lo_p   (),
		.e1_rx_lo_n   (),
		.e1_tx_hi     (),
		.e1_tx_lo     (),
		.e1_rx_data   ({e1B_rx_data, e1A_rx_data}),
		.e1_rx_clk    ({e1B_rx_clk,  e1A_rx_clk }),
		.e1_tx_data   (),
		.e1_tx_clk    (),
		.usb_dp       (usb_dp),
		.usb_dn       (usb_dn),
		.usb_pu       (usb_pu),
		.flash_mosi_i (flash_mosi_i),
		.flash_mosi_o (flash_mosi_o),
		.flash_mosi_oe(flash_mosi_oe),
		.flash_miso_i (flash_miso_i),
		.flash_miso_o (flash_miso_o),
		.flash_miso_oe(flash_miso_oe),
		.flash_clk_i  (flash_clk_i),
		.flash_clk_o  (flash_clk_o),
		.flash_clk_oe (flash_clk_oe),
		.flash_csn_o  (flash_csn_o),
		.dbg_rx       (1'b1),
		.dbg_tx       (dbg_tx),
		.rgb          (rgb),
		.wb_m_addr    (wb_addr),
		.wb_m_rdata   (wb_rdata_flat),
		.wb_m_wdata   (wb_wdata),
		.wb_m_wmsk    (wb_wmsk),
		.wb_m_we      (wb_we),
		.wb_m_cyc     (wb_cyc),
		.wb_m_ack     (wb_ack),
		.tick_e1_rx   (tick_e1_rx),
		.tick_usb_sof (tick_usb_sof),
		.clk_sys      (clk_sys),
		.rst_sys      (rst_sys),
		.clk_48m      (clk_48m),
		.rst_48m      (rst_48m)
	);

	// WB read data flattening
	for (i=0; i<WB_N; i=i+1)
		assign wb_rdata_flat[i*32+:32] = wb_rdata[i];

	// SPI IO
	SB_IO #(
		.PIN_TYPE(6'b101001),
		.PULLUP(1'b1)
	) spi_io_I[2:0] (
		.PACKAGE_PIN  ({flash_mosi,    flash_miso,    flash_clk   }),
		.OUTPUT_ENABLE({flash_mosi_oe, flash_miso_oe, flash_clk_oe}),
		.D_OUT_0      ({flash_mosi_o,  flash_miso_o,  flash_clk_o }),
		.D_IN_0       ({flash_mosi_i,  flash_miso_i,  flash_clk_i })
	);

	assign flash_cs_n = flash_csn_o;


	// Misc [0]
	// ----

	misc misc_I (
		.btn          (btn),
		.tick_e1_rx   (tick_e1_rx),
		.tick_usb_sof (tick_usb_sof),
		.rst_req      (rst_req),
		.wb_addr      (wb_addr[7:0]),
		.wb_rdata     (wb_rdata[0]),
		.wb_wdata     (wb_wdata),
		.wb_we        (wb_we),
		.wb_cyc       (wb_cyc[0]),
		.wb_ack       (wb_ack[0]),
		.clk          (clk_sys),
		.rst          (rst_sys)
	);


	// LIU SPI [1]
	// -------

	ice40_spi_wb #(
		.N_CS(2),
		.WITH_IOB(1),
		.UNIT(1)
	) spi_I (
		.pad_mosi (liu_mosi),
		.pad_miso (liu_miso),
		.pad_clk  (liu_clk),
		.pad_csn  (liu_cs_n),
		.wb_addr  (wb_addr[3:0]),
		.wb_rdata (wb_rdata[1]),
		.wb_wdata (wb_wdata),
		.wb_we    (wb_we),
		.wb_cyc   (wb_cyc[1]),
		.wb_ack   (wb_ack[1]),
		.clk      (clk_sys),
		.rst      (rst_sys)
	);


	// Vio PDM
	// -------

	// Compat with iCEpick
	assign vio_pdm = 1'b1;


	// Clock / Reset
	// -------------

	sysmgr sys_mgr_I (
		.clk_in (clk_in),
		.rst_in (rst_req),
		.clk_sys(clk_sys),
		.rst_sys(rst_sys),
		.clk_48m(clk_48m),
		.rst_48m(rst_48m)
	);

endmodule // top
