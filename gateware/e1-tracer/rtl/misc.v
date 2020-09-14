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

module misc (
	// Button
	input  wire        btn,

	// Ticks
	input  wire  [1:0] tick_e1_rx,
	input  wire        tick_usb_sof,

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

	genvar i;

	// Bus
	wire bus_clr;
	reg  bus_we_boot;

	// Counters
	reg  [15:0] cnt_e1_rx[0:1];
	reg  [15:0] cap_e1_rx[0:1];
	reg  [31:0] cnt_time;

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
		if (bus_clr | ~wb_we)
			bus_we_boot <= 1'b0;
		else
			bus_we_boot <= wb_addr == 4'h0;

	// Read mux
	always @(posedge clk)
		if (bus_clr)
			wb_rdata <= 32'h00000000;
		else
			case (wb_addr[3:0])
				4'h4:    wb_rdata <= { cap_e1_rx[0], 16'h0000 };
				4'h5:    wb_rdata <= { cap_e1_rx[1], 16'h0000 };
				4'h7:    wb_rdata <= cnt_time;
				default: wb_rdata <= 32'hxxxxxxxx;
			endcase


	// Counters
	// --------

	// E1 ticks
	for (i=0; i<2; i=i+1) begin

		always @(posedge clk or posedge rst)
			if (rst)
				cnt_e1_rx[i] <= 16'h0000;
			else if (tick_e1_rx[i])
				cnt_e1_rx[i] <= cnt_e1_rx[i] + 1;

		always @(posedge clk)
			if (tick_usb_sof)
				cap_e1_rx[i] <= cnt_e1_rx[i];

	end

	// Time counter
	always @(posedge clk)
		if (rst)
			cnt_time <= 32'h00000000;
		else
			cnt_time <= cnt_time + 1;


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
