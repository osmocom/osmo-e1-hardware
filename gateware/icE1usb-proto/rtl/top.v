/*
 * top.v
 *
 * vim: ts=4 sw=4
 *
 * Top-level for the icE1usb icebreaker/bitsy based prototypes
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: CERN-OHL-S-2.0
 */

`default_nettype none

module top (
	// E1 PHY
	input  wire e1_rx_hi_p,
//	input  wire e1_rx_hi_n,
	input  wire e1_rx_lo_p,
//	input  wire e1_rx_lo_n,

	output wire e1_tx_hi,
	output wire e1_tx_lo,

	output wire e1_vref_ct_pdm,
	output wire e1_vref_p_pdm,
	output wire e1_vref_n_pdm,

	// USB
	inout  wire usb_dp,
	inout  wire usb_dn,
	output wire usb_pu,

	// Flash
	inout  wire flash_mosi,
	inout  wire flash_miso,
	inout  wire flash_clk,
	inout  wire flash_cs_n,

	// Button
	input  wire btn,

	// Clock (30.72 MHz)
	input  wire clk_in,
	output wire clk_tune_hi,
	output wire clk_tune_lo,

	// Debug UART
	input  wire dbg_rx,
	output wire dbg_tx,

	// RGB LEDs
	output wire [2:0] rgb
);

	localparam integer WB_N = 1;

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
	wire tick_e1_rx;
	wire tick_e1_tx;
	wire tick_usb_sof;

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
		.E1_N(1),
		.E1_UNIT_HAS_RX(1'b1),
		.E1_UNIT_HAS_TX(1'b1),
		.E1_LIU(0)
	) soc_I (
		.e1_rx_hi_p   (e1_rx_hi_p),
//		.e1_rx_hi_n   (e1_rx_hi_n),
		.e1_rx_lo_p   (e1_rx_lo_p),
//		.e1_rx_lo_n   (e1_rx_lo_n),
		.e1_tx_hi     (e1_tx_hi),
		.e1_tx_lo     (e1_tx_lo),
		.e1_rx_data   (),
		.e1_rx_clk    (),
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
		.dbg_rx       (dbg_rx),
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
		.tick_e1_tx   (tick_e1_tx),
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
		.e1_vref_ct_pdm(e1_vref_ct_pdm),
		.e1_vref_p_pdm (e1_vref_p_pdm),
		.e1_vref_n_pdm (e1_vref_n_pdm),
		.clk_tune_hi   (clk_tune_hi),
		.clk_tune_lo   (clk_tune_lo),
		.btn           (btn),
		.tick_e1_rx    (tick_e1_rx),
		.tick_e1_tx    (tick_e1_tx),
		.tick_usb_sof  (tick_usb_sof),
		.rst_req       (rst_req),
		.wb_addr       (wb_addr[7:0]),
		.wb_rdata      (wb_rdata[0]),
		.wb_wdata      (wb_wdata),
		.wb_we         (wb_we),
		.wb_cyc        (wb_cyc[0]),
		.wb_ack        (wb_ack[0]),
		.clk           (clk_sys),
		.rst           (rst_sys)
	);


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
