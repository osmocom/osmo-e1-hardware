/*
 * sysmgr.v
 *
 * vim: ts=4 sw=4
 *
 * System Clock / Reset generation
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: CERN-OHL-S-2.0
 */

`default_nettype none

module sysmgr (
	input  wire clk_in,
	input  wire rst_in,
	output wire clk_sys,
	output wire rst_sys,
	output wire clk_48m,
	output wire rst_48m
);

	// Signals
	wire pll_lock;
	wire pll_reset_n;

	wire clk_12m_i;
	wire clk_24m_i;
	wire clk_48m_i;

	wire rst_i;
	wire rst_out;
	reg [3:0] rst_cnt;

	// Global input buffer for 12 MHz clock
	SB_GB_IO #(
		.PIN_TYPE(6'b000001)
	) gb_in (
		.PACKAGE_PIN(clk_in),
		.GLOBAL_BUFFER_OUTPUT(clk_12m_i)
	);

	// PLL instance
	SB_PLL40_2F_CORE #(
		.DIVR(4'b0000),
		.DIVF(7'b0111111),
		.DIVQ(3'b100),
		.FILTER_RANGE(3'b001),
		.FEEDBACK_PATH("SIMPLE"),
		.DELAY_ADJUSTMENT_MODE_FEEDBACK("FIXED"),
		.FDA_FEEDBACK(4'b0000),
		.SHIFTREG_DIV_MODE(2'b00),
		.PLLOUT_SELECT_PORTA("GENCLK"),
		.PLLOUT_SELECT_PORTB("GENCLK_HALF"),
		.ENABLE_ICEGATE_PORTA(1'b0),
		.ENABLE_ICEGATE_PORTB(1'b0)
	) pll_I (
		.REFERENCECLK(clk_12m_i),
		.PLLOUTCOREA(),
		.PLLOUTGLOBALA(clk_48m_i),
		.PLLOUTCOREB(),
		.PLLOUTGLOBALB(clk_24m_i),
		.EXTFEEDBACK(1'b0),
		.DYNAMICDELAY(8'h00),
		.RESETB(pll_reset_n),
		.BYPASS(1'b0),
		.LATCHINPUTVALUE(1'b0),
		.LOCK(pll_lock),
		.SDI(1'b0),
		.SDO(),
		.SCLK(1'b0)
	);

	assign clk_sys = clk_24m_i;
	assign clk_48m = clk_48m_i;

	// PLL reset generation
	assign pll_reset_n = ~rst_in;

	// Logic reset generation
	always @(posedge clk_24m_i or negedge pll_lock)
		if (!pll_lock)
			rst_cnt <= 4'h0;
		else if (~rst_cnt[3])
			rst_cnt <= rst_cnt + 1;

	assign rst_i = ~rst_cnt[3];

	SB_GB rst_gbuf_I (
		.USER_SIGNAL_TO_GLOBAL_BUFFER(rst_i),
		.GLOBAL_BUFFER_OUTPUT(rst_out)
	);

	assign rst_sys = rst_out;
	assign rst_48m = rst_out;

endmodule // sysmgr
