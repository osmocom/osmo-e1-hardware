/*
 * capcnt.v
 *
 * Simple capture/counter blocks
 *
 * vim: ts=4 sw=4
 *
 * Copyright (C) 2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: CERN-OHL-P-2.0
 */

`define WITH_SB_MAC16

module capcnt #(
	parameter integer W = 16,
)(
	output wire [W-1:0] cnt_cur,
	output wire [W-1:0] cnt_cap,
	input  wire         inc,
	input  wire         cap,
	input  wire         clk,
	input  wire         rst
);

`ifdef WITH_SB_MAC16
	generate

		if (W == 16)
			capcnt16_sb_mac16 sub_I (
				.cnt_cur (cnt_cur),
				.cnt_cap (cnt_cap),
				.inc     (inc),
				.cap     (cap),
				.clk     (clk),
				.rst     (rst)
			);

		else if (W == 32)
			capcnt32_sb_mac16 sub_I (
				.cnt_cur (cnt_cur),
				.cnt_cap (cnt_cap),
				.inc     (inc),
				.cap     (cap),
				.clk     (clk),
				.rst     (rst)
			);

	endgenerate
`else

	reg [W-1:0] cnt_cur_i;
	reg [W-1:0] cnt_cap_i;

	always @(posedge clk)
		if (rst)
			cnt_cur_i <= 0;
		else if (inc)
			cnt_cur_i <= cnt_cur_i + 1;

	always @(posedge clk)
		if (rst)
			cnt_cap_i <= 0;
		else if (cap)
			cnt_cap_i <= cnt_cur_i;

	assign cnt_cur = cnt_cur_i;
	assign cnt_cap = cnt_cap_i;

`endif

endmodule // capcnt
