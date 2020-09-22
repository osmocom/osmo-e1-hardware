/*
 * led_blinker.v
 *
 * vim: ts=4 sw=4
 *
 * Controls E1 led blinking
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: CERN-OHL-S-2.0
 */

`default_nettype none

`define SMALL

module led_blinker (
	// Requested LED state
	input  wire [7:0] led_state,

	// Shift Register interface
	output wire [7:0] sr_val,
	output reg        sr_go,
	input  wire       sr_rdy,

	// Clock / Reset
	input  wire clk,
	input  wire rst
);

	// ff00 f0f0 cccc aaaa
	localparam integer BLINK_SLOW_SPEED   = 0;
	localparam  [15:0] BLINK_SLOW_PATTERN = 16'hf0f0;
	localparam integer BLINK_FAST_SPEED   = 0;
	localparam  [15:0] BLINK_FAST_PATTERN = 16'haaaa;


	// Signals
	// -------

	reg  [15:0] tick_cnt;
	wire        tick;

	reg  [ 9:0] cycle;

	wire        blink_slow;
	wire        blink_fast;

	reg  [ 3:0] led;


	// Counter
	// -------

	// Tick
	always @(posedge clk)
`ifdef SMALL
		tick_cnt <= (tick_cnt + 1) & (tick ? 16'h0000 : 16'hffff);
`else
		tick_cnt <= tick ? 16'h00000 : (tick_cnt+ 1);
`endif

	assign tick = tick_cnt[15];

	// Cycles
	always @(posedge clk)
		cycle <= cycle + tick;


	// Blink patterns
	// --------------

	// Base
	assign blink_slow = BLINK_SLOW_PATTERN[cycle[9-BLINK_SLOW_SPEED:6-BLINK_SLOW_SPEED]];
	assign blink_fast = BLINK_FAST_PATTERN[cycle[9-BLINK_FAST_SPEED:6-BLINK_FAST_SPEED]];

	// Per-led
	always @(*)
	begin : led_state_proc
		integer i;
		for (i=0; i<4; i=i+1)
			led[i] = led_state[2*i+1] ? (led_state[2*i] ? blink_fast : blink_slow) : led_state[2*i];
	end


	// Request
	// -------

	// Keep  update requests 'pending' for half a cycle
	always @(posedge clk or posedge rst)
		if (rst)
			sr_go <= 1'b0;
		else
			sr_go <= (sr_go & ~(sr_rdy | tick_cnt[14])) | tick;

	assign sr_val = {
		led[1],
		1'b0,
		led[0],
		1'b0,
		1'b0,
		led[2],
		1'b0,
		led[3]
	};

endmodule // led_blinker
