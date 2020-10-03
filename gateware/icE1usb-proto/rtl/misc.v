/*
 * misc.v
 *
 * vim: ts=4 sw=4
 *
 * Misc peripheral functions
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: CERN-OHL-S-2.0
 */

`default_nettype none

// `define WITH_PDM_READBACK

module misc (
	// PDM outputs
	output wire e1_vref_ct_pdm,
	output wire e1_vref_p_pdm,
	output wire e1_vref_n_pdm,

	output wire clk_tune_hi,
	output wire clk_tune_lo,

	// Button
	input  wire btn,

	// Ticks
	input  wire tick_e1_rx,
	input  wire tick_e1_tx,
	input  wire tick_usb_sof,

	// Reset request
	output wire        rst_req,

	// Wishbone
	input  wire [ 7:0] wb_addr,
	output reg  [31:0] wb_rdata,
	input  wire [31:0] wb_wdata,
	input  wire        wb_we,
	input  wire        wb_cyc,
	output reg         wb_ack,

	// Clock / Reset
	input  wire clk,
	input  wire rst
);

	// Signals
	// -------

	// Bus
	wire        bus_clr;
	reg         bus_we_boot;
	reg  [ 1:0] bus_we_pdm_clk;
	reg  [ 2:0] bus_we_pdm_e1;

	// Counters
	wire [15:0] cap_e1_rx;
	wire [15:0] cap_e1_tx;
	wire [31:0] cnt_time;

	// PDM
	reg  [12:0] pdm_clk[0:1];
	reg  [ 8:0] pdm_e1[0:2];

	// Boot
	reg   [1:0] boot_sel;
	reg         boot_now;


	// Bus interface
	// -------------

	// Ack
	always @(posedge clk)
		wb_ack <= wb_cyc & ~wb_ack;

	assign bus_clr = ~wb_cyc | wb_ack;

	// Write enables
	always @(posedge clk)
		if (bus_clr | ~wb_we) begin
			bus_we_boot       <= 1'b0;
			bus_we_pdm_clk[0] <= 1'b0;
			bus_we_pdm_clk[1] <= 1'b0;
			bus_we_pdm_e1[0]  <= 1'b0;
			bus_we_pdm_e1[1]  <= 1'b0;
			bus_we_pdm_e1[2]  <= 1'b0;
		end else begin
			bus_we_boot       <= wb_addr == 4'h0;
			bus_we_pdm_clk[0] <= wb_addr == 4'h8;
			bus_we_pdm_clk[1] <= wb_addr == 4'h9;
			bus_we_pdm_e1[0]  <= wb_addr == 4'ha;
			bus_we_pdm_e1[1]  <= wb_addr == 4'hb;
			bus_we_pdm_e1[2]  <= wb_addr == 4'hc;
		end

	// Read mux
	always @(posedge clk)
		if (bus_clr)
			wb_rdata <= 32'h00000000;
		else
			case (wb_addr[3:0])
				4'h4:    wb_rdata <= { cap_e1_tx, cap_e1_rx };
				4'h7:    wb_rdata <= cnt_time;
`ifdef WITH_PDM_READBACK
				4'h8:    wb_rdata <= { pdm_clk[0][12], 19'h00000, pdm_clk[0][11:0] };
				4'h9:    wb_rdata <= { pdm_clk[1][12], 19'h00000, pdm_clk[1][11:0] };
				4'ha:    wb_rdata <= {  pdm_e1[0][8], 23'h000000,  pdm_e1[0][ 7:0] };
				4'hb:    wb_rdata <= {  pdm_e1[1][8], 23'h000000,  pdm_e1[1][ 7:0] };
				4'hc:    wb_rdata <= {  pdm_e1[2][8], 23'h000000,  pdm_e1[2][ 7:0] };
`endif
				default: wb_rdata <= 32'hxxxxxxxx;
			endcase


	// Counters
	// --------

	// E1 ticks
	capcnt #(
		.W(16)
	) e1_cnt_I[1:0] (
		.cnt_cur (),
		.cnt_cap ({cap_e1_tx,  cap_e1_rx }),
		.inc     ({tick_e1_tx, tick_e1_rx}),
		.cap     (tick_usb_sof),
		.clk     (clk),
		.rst     (rst)
	);

	// Time
	capcnt #(
		.W(32)
	) time_cnt_I (
		.cnt_cur (cnt_time),
		.cnt_cap (),
		.inc     (1'b1),
		.cap     (1'b0),
		.clk     (clk),
		.rst     (rst)
	);


	// PDM outputs
	// -----------

	// Registers
	always @(posedge clk or posedge rst)
		if (rst) begin
			pdm_clk[0] <= 0; // 13'h1800;
			pdm_clk[1] <= 0; // 13'h1800;
			pdm_e1[0]  <= 0; // 9'h190;
			pdm_e1[1]  <= 0; // 9'h190;
			pdm_e1[2]  <= 0; // 9'h190;
		end else begin
			if (bus_we_pdm_clk[0])  pdm_clk[0] <= { wb_wdata[31], wb_wdata[11:0] };
			if (bus_we_pdm_clk[1])  pdm_clk[1] <= { wb_wdata[31], wb_wdata[11:0] };
			if (bus_we_pdm_e1[0])   pdm_e1[0]  <= { wb_wdata[31], wb_wdata[ 7:0] };
			if (bus_we_pdm_e1[1])   pdm_e1[1]  <= { wb_wdata[31], wb_wdata[ 7:0] };
			if (bus_we_pdm_e1[2])   pdm_e1[2]  <= { wb_wdata[31], wb_wdata[ 7:0] };
		end

	// PDM cores
	pdm #(
		.WIDTH(12),
		.PHY("ICE40"),
		.DITHER("YES")
	) pdm_clk_I[1:0] (
		.pdm    ({ clk_tune_hi,      clk_tune_lo      }),
		.cfg_val({ pdm_clk[1][11:0], pdm_clk[0][11:0] }),
		.cfg_oe ({ pdm_clk[1][12],   pdm_clk[0][12]   }),
		.clk    (clk),
		.rst    (rst)
	);

	pdm #(
		.WIDTH(8),
		.PHY("ICE40"),
		.DITHER("NO")
	) pdm_e1_I[2:0] (
		.pdm    ({ e1_vref_ct_pdm, e1_vref_p_pdm,  e1_vref_n_pdm  }),
		.cfg_val({ pdm_e1[2][7:0], pdm_e1[1][7:0], pdm_e1[0][7:0] }),
		.cfg_oe ({ pdm_e1[2][8],   pdm_e1[1][8],   pdm_e1[0][8]   }),
		.clk    (clk),
		.rst    (rst)
	);


	// DFU / Reboot
	// ------------

	always @(posedge clk or posedge rst)
		if (rst) begin
			boot_now <= 1'b0;
			boot_sel <= 2'b00;
		end else if (bus_we_boot) begin
			boot_now <= wb_wdata[2];
			boot_sel <= wb_wdata[1:0];
		end

	dfu_helper #(
		.TIMER_WIDTH(26),
		.BTN_MODE(3),
		.DFU_MODE(0)
	) dfu_I (
		.boot_sel(boot_sel),
		.boot_now(boot_now),
		.btn_pad (btn),
		.btn_val (),
		.rst_req (rst_req),
		.clk     (clk),
		.rst     (rst)
	);

endmodule // misc
