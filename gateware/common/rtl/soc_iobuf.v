/*
 * soc_iobuf.v
 *
 * vim: ts=4 sw=4
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: CERN-OHL-S-2.0
 */

`default_nettype none

module soc_iobuf (
	// Wishbone slave (from CPU)
	input  wire [15:0] wb_cpu_addr,
	output wire [31:0] wb_cpu_rdata,
	input  wire [31:0] wb_cpu_wdata,
	input  wire [ 3:0] wb_cpu_wmsk,
	input  wire        wb_cpu_we,
	input  wire [ 2:0] wb_cpu_cyc,	// 0=EP buf, 1=SPRAM, 2=DMA
	output wire [ 2:0] wb_cpu_ack,

	// Wishbone slave (from E1)
	input  wire [13:0] wb_e1_addr,
	output wire [31:0] wb_e1_rdata,
	input  wire [31:0] wb_e1_wdata,
	input  wire [ 3:0] wb_e1_wmsk,
	input  wire        wb_e1_we,
	input  wire        wb_e1_cyc,
	output wire        wb_e1_ack,

	// USB EP-Buf master
	output wire [ 8:0] ep_tx_addr_0,
	output wire [31:0] ep_tx_data_0,
	output wire        ep_tx_we_0,

	output wire [ 8:0] ep_rx_addr_0,
	input  wire [31:0] ep_rx_data_1,
	output wire        ep_rx_re_0,

	/* Clock / Reset */
	input  wire clk,
	input  wire rst
);

	// Signals
	// -------

	// SPRAM
	wire [13:0] spr_addr;
	wire [31:0] spr_rdata;
	wire [31:0] spr_wdata;
	wire [ 3:0] spr_wmsk;
	wire        spr_we;
	wire        spr_cyc;
	wire        spr_ack;

	wire [13:0] spr0_addr;
	wire [31:0] spr0_rdata;
	wire [31:0] spr0_wdata;
	wire [ 3:0] spr0_wmsk;
	wire        spr0_we;
	wire        spr0_cyc;
	wire        spr0_ack;

	wire [13:0] spr1_addr;
	wire [31:0] spr1_rdata;
	wire [31:0] spr1_wdata;
	wire [ 3:0] spr1_wmsk;
	wire        spr1_we;
	wire        spr1_cyc;
	wire        spr1_ack;

	wire [13:0] spr2_addr;
	wire [31:0] spr2_rdata;
	wire [31:0] spr2_wdata;
	wire [ 3:0] spr2_wmsk;
	wire        spr2_we;
	wire        spr2_cyc;
	wire        spr2_ack;

	// EP Buffer
	wire [ 8:0] epb_addr;
	wire [31:0] epb_rdata;
	wire [31:0] epb_wdata;
	wire        epb_we;
	wire        epb_cyc;
	wire        epb_ack;

	wire [ 8:0] epb0_addr;
	wire [31:0] epb0_rdata;
	wire [31:0] epb0_wdata;
	wire        epb0_we;
	wire        epb0_cyc;
	wire        epb0_ack;

	wire [ 8:0] epb1_addr;
	wire [31:0] epb1_rdata;
	wire [31:0] epb1_wdata;
	wire        epb1_we;
	wire        epb1_cyc;
	wire        epb1_ack;

	// DMA
	wire [31:0] wb_rdata_dma;


	// SPRAM
	// -----

	// Instance
	ice40_spram_wb #(
		.DW(32),
		.AW(14),
		.ZERO_RDATA(0)
	) spram_I (
		.wb_addr (spr_addr),
		.wb_rdata(spr_rdata),
		.wb_wdata(spr_wdata),
		.wb_wmsk (spr_wmsk),
		.wb_we   (spr_we),
		.wb_cyc  (spr_cyc),
		.wb_ack  (spr_ack),
		.clk     (clk),
		.rst     (rst)
	);

	// Arbiter
	wb_arbiter #(
		.N(3),
		.DW(32),
		.AW(14)
	) spram_arb_I (
		.s_addr ({spr2_addr,  spr1_addr,  spr0_addr}),
		.s_rdata({spr2_rdata, spr1_rdata, spr0_rdata}),
		.s_wdata({spr2_wdata, spr1_wdata, spr0_wdata}),
		.s_wmsk ({spr2_wmsk,  spr1_wmsk,  spr0_wmsk}),
		.s_we   ({spr2_we,    spr1_we,    spr0_we}),
		.s_cyc  ({spr2_cyc,   spr1_cyc,   spr0_cyc}),
		.s_ack  ({spr2_ack,   spr1_ack,   spr0_ack}),
		.m_addr (spr_addr),
		.m_rdata(spr_rdata),
		.m_wdata(spr_wdata),
		.m_wmsk (spr_wmsk),
		.m_we   (spr_we),
		.m_cyc  (spr_cyc),
		.m_ack  (spr_ack),
		.clk    (clk),
		.rst    (rst)
	);


	// EP buffer
	// ---------

	// Instance
	wb_epbuf #(
		.AW(9),
		.DW(32)
	) epbuf_I (
		.wb_addr     (epb_addr),
		.wb_rdata    (epb_rdata),
		.wb_wdata    (epb_wdata),
		.wb_we       (epb_we),
		.wb_cyc      (epb_cyc),
		.wb_ack      (epb_ack),
		.ep_tx_addr_0(ep_tx_addr_0),
		.ep_tx_data_0(ep_tx_data_0),
		.ep_tx_we_0  (ep_tx_we_0),
		.ep_rx_addr_0(ep_rx_addr_0),
		.ep_rx_data_1(ep_rx_data_1),
		.ep_rx_re_0  (ep_rx_re_0),
		.clk         (clk),
		.rst         (rst)
	);

	// Arbiter
	wb_arbiter #(
		.N(2),
		.DW(32),
		.AW(9)
	) epbam_arb_I (
		.s_addr ({epb1_addr,  epb0_addr}),
		.s_rdata({epb1_rdata, epb0_rdata}),
		.s_wdata({epb1_wdata, epb0_wdata}),
		.s_wmsk (8'hff),
		.s_we   ({epb1_we,   epb0_we}),
		.s_cyc  ({epb1_cyc,  epb0_cyc}),
		.s_ack  ({epb1_ack,  epb0_ack}),
		.m_addr (epb_addr),
		.m_rdata(epb_rdata),
		.m_wdata(epb_wdata),
		.m_we   (epb_we),
		.m_cyc  (epb_cyc),
		.m_ack  (epb_ack),
		.clk    (clk),
		.rst    (rst)
	);


	// DMA
	// ---

	wb_dma #(
		.A0W(14),
		.A1W(9),
		.DW(32)
	) dma_I (
		.m0_addr  (spr2_addr),
		.m0_rdata (spr2_rdata),
		.m0_wdata (spr2_wdata),
		.m0_we    (spr2_we),
		.m0_cyc   (spr2_cyc),
		.m0_ack   (spr2_ack),
		.m1_addr  (epb1_addr),
		.m1_rdata (epb1_rdata),
		.m1_wdata (epb1_wdata),
		.m1_we    (epb1_we),
		.m1_cyc   (epb1_cyc),
		.m1_ack   (epb1_ack),
		.ctl_addr (wb_cpu_addr[1:0]),
		.ctl_rdata(wb_rdata_dma),
		.ctl_wdata(wb_cpu_wdata),
		.ctl_we   (wb_cpu_we),
		.ctl_cyc  (wb_cpu_cyc[2]),
		.ctl_ack  (wb_cpu_ack[2]),
		.clk      (clk),
		.rst      (rst)
	);

	assign spr2_wmsk = 4'h0;


	// External accesses
	// -----------------

	// CPU
	assign spr1_addr     = wb_cpu_addr[13:0];
	assign spr1_wdata    = wb_cpu_wdata;
	assign spr1_wmsk     = wb_cpu_wmsk;
	assign spr1_we       = wb_cpu_we;
	assign spr1_cyc      = wb_cpu_cyc[1];
	assign wb_cpu_ack[1] = spr1_ack;

	assign epb0_addr     = wb_cpu_addr[8:0];
	assign epb0_wdata    = wb_cpu_wdata;
	assign epb0_we       = wb_cpu_we;
	assign epb0_cyc      = wb_cpu_cyc[0];
	assign wb_cpu_ack[0] = epb0_ack;

	assign wb_cpu_rdata = spr1_rdata | epb0_rdata | wb_rdata_dma;

	// E1
	assign spr0_addr   = wb_e1_addr;
	assign spr0_wdata  = wb_e1_wdata;
	assign spr0_wmsk   = wb_e1_wmsk;
	assign spr0_we     = wb_e1_we;
	assign spr0_cyc    = wb_e1_cyc;
	assign wb_e1_rdata = spr0_rdata;
	assign wb_e1_ack   = spr0_ack;

endmodule // soc_iobuf
