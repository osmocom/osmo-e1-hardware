/*
 * sr_btn_if.v
 *
 * vim: ts=4 sw=4
 *
 * Combined SPI + Shift Register + Button input interface
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: CERN-OHL-S-2.0
 */

`default_nettype none

`define SMALL

module sr_btn_if #(
	parameter integer TICK_LOG2_DIV = 3
)(
	// Pads
	output wire flash_mosi,
	input  wire flash_miso,
	output wire flash_clk,
	output wire flash_cs_n,

	inout  wire e1_led_rclk,

	// SPI module interface
	output wire spi_mosi_i,
	input wire  spi_mosi_o,
	input wire  spi_mosi_oe,

	output wire spi_miso_i,
	input wire  spi_miso_o,
	input wire  spi_miso_oe,

	output wire spi_clk_i,
	input wire  spi_clk_o,
	input wire  spi_clk_oe,

	input wire  spi_csn_o,
	input wire  spi_csn_oe,

	// SPI muxing req/gnt
	input  wire spi_req,
	output wire spi_gnt,

	// Shift Register interface
	input  wire [7:0] sr_val,
	input  wire       sr_go,
	output wire       sr_rdy,

	// Button status
	output reg  btn_val,
	output reg  btn_stb,

	// Clock / Reset
	input  wire clk,
	input  wire rst
);

	// Signals
	// -------

	// FSM
	localparam
		ST_IDLE     = 0,
		ST_SPI      = 1,
		ST_SHIFT_LO = 2,
		ST_SHIFT_HI = 3,
		ST_LATCH    = 4,
		ST_SENSE    = 5,
		ST_PAUSE    = 6;

	reg [2:0] state;
	reg [2:0] state_nxt;

	// Shift Register IO
	reg  srio_clk_o;
	reg  srio_dat_o;
	reg  srio_rclk_o;
	reg  srio_rclk_oe;
	wire srio_rclk_i;

	// SPI IO
	reg  sio_sel;

	wire sio_miso_o, sio_miso_oe, sio_miso_i;
	wire sio_mosi_o, sio_mosi_oe, sio_mosi_i;
	wire sio_clk_o,  sio_clk_oe,  sio_clk_i;
	wire sio_csn_o, sio_csn_oe;

	// Input synchronizer
	reg [1:0] btn_sync;

	// Counters
	reg [TICK_LOG2_DIV:0] tick_cnt;
	wire tick;

	reg [3:0] bit_cnt;
	wire      bit_cnt_last;

	reg [3:0] sense_cnt_in;
	reg [3:0] sense_cnt;
	wire      sense_cnt_last;

	// Shift register
	reg [7:0] shift_data;


	// FSM
	// ---

	// State register
	always @(posedge clk)
		if (rst)
			state <= ST_IDLE;
		else
			state <= state_nxt;

	// Next-State
	always @(*)
	begin
		// Default
		state_nxt = state;

		// Change conditions
		case (state)
		ST_IDLE:
			if (sr_go)
				state_nxt = ST_SHIFT_LO;
			else if (spi_req)
				state_nxt = ST_SPI;

		ST_SPI:
			if (~spi_req)
				state_nxt = ST_IDLE;

		ST_SHIFT_LO:
			if (tick)
				state_nxt = ST_SHIFT_HI;

		ST_SHIFT_HI:
			if (tick)
				state_nxt = bit_cnt_last ? ST_LATCH : ST_SHIFT_LO;

		ST_LATCH:
			if (tick)
				state_nxt = ST_SENSE;

		ST_SENSE:
			if (tick & sense_cnt_last)
				state_nxt = ST_PAUSE;

		ST_PAUSE:
			if (tick)
				state_nxt = ST_IDLE;
		endcase
	end


	// IO pads
	// -------

	// Muxing & Sharing
	assign spi_mosi_i = sio_mosi_i;
	assign spi_miso_i = sio_miso_i;
	assign spi_clk_i  = sio_clk_i;

	assign sio_mosi_o  = sio_sel ? spi_mosi_o  : srio_dat_o;
	assign sio_mosi_oe = sio_sel ? spi_mosi_oe : 1'b1;
	assign sio_miso_o  = sio_sel ? spi_miso_o  : 1'b0;
	assign sio_miso_oe = sio_sel ? spi_miso_oe : 1'b0;
	assign sio_clk_o   = sio_sel ? spi_clk_o   : srio_clk_o;
	assign sio_clk_oe  = sio_sel ? spi_clk_oe  : 1'b1;
	assign sio_csn_o   = sio_sel ? spi_csn_o   : 1'b1;
	assign sio_csn_oe  = sio_sel ? spi_csn_oe  : 1'b1;

	// MOSI / MISO / SCK / RCLK
	SB_IO #(
		.PIN_TYPE(6'b101001),
		.PULLUP(1'b1)
	) iob_I[3:0] (
		.PACKAGE_PIN  ({flash_mosi,  flash_miso,  flash_clk,  e1_led_rclk}),
		.OUTPUT_ENABLE({sio_mosi_oe, sio_miso_oe, sio_clk_oe, srio_rclk_oe}),
		.D_OUT_0      ({sio_mosi_o,  sio_miso_o,  sio_clk_o,  srio_rclk_o}),
		.D_IN_0       ({sio_mosi_i,  sio_miso_i,  sio_clk_i,  srio_rclk_i})
	);

	// Bypass OE for CS_n line
	assign flash_cs_n = sio_csn_o;


	// SPI grant
	// ---------

	// Mux select
	always @(posedge clk)
		sio_sel <= (state_nxt == ST_SPI);

	assign spi_gnt = sio_sel;


	// Button input synchronizer
	// -------------------------

	always @(posedge clk)
		btn_sync <= { btn_sync[0], srio_rclk_i };


	// Control
	// -------

	// Tick counter
	always @(posedge clk)
`ifdef SMALL
		tick_cnt <= { (TICK_LOG2_DIV+1){ (~tick & (state != ST_IDLE)) } } & (tick_cnt + 1);
`else
		if (state == ST_IDLE)
			tick_cnt <= 0;
		else
			tick_cnt <= tick ? 0 : (tick_cnt + 1);
`endif

	assign tick = tick_cnt[TICK_LOG2_DIV];

	// Bit counter
	always @(posedge clk)
`ifdef SMALL
		bit_cnt <= { 4{state != ST_IDLE} } & (bit_cnt + (tick & (state == ST_SHIFT_LO)));
`else
		if (state == ST_IDLE)
			bit_cnt <= 4'h0;
		else if (tick & (state == ST_SHIFT_LO))
			bit_cnt <= bit_cnt + 1;
`endif

	assign bit_cnt_last = bit_cnt[3];

	// Sense counters
`ifdef SMALL
	(* keep *) wire state_is_not_latch = (state != ST_LATCH);
`endif
	always @(posedge clk)
`ifdef SMALL
	begin
		sense_cnt    <= { 4{state_is_not_latch} } & (sense_cnt    + tick);
		sense_cnt_in <= { 4{state_is_not_latch} } & (sense_cnt_in + (tick & btn_sync[1]));
	end
`else
		if (state == ST_LATCH) begin
			sense_cnt    <= 0;
			sense_cnt_in <= 0;
		end else if (tick) begin
			sense_cnt    <= sense_cnt + 1;
			sense_cnt_in <= sense_cnt_in + btn_sync[1];
		end
`endif

	assign sense_cnt_last = sense_cnt[3];

	// Decision
	always @(posedge clk)
`ifdef SMALL
		btn_val <= ((state == ST_PAUSE) & (sense_cnt_in[3:2] == 2'b00)) | ((state != ST_PAUSE) & btn_val);
`else
		if (state == ST_PAUSE)
			btn_val <= (sense_cnt_in[3:2] == 2'b00);
`endif

	always @(posedge clk)
		btn_stb <= (state == ST_PAUSE) & tick;

	// Data shift register
`ifdef SMALL
	wire [7:0] m = { 8{ sr_go & sr_rdy } };
	wire [7:0] shift_data_nxt = (sr_val & m) | ({ shift_data[6:0], 1'b0 } & ~m);
	wire shift_ce = (sr_go & sr_rdy) | (tick & (state == ST_SHIFT_HI));

	always @(posedge clk)
		if (shift_ce)
			shift_data <= shift_data_nxt;
`else
	always @(posedge clk)
		if (sr_go & sr_rdy)
			shift_data <= sr_val;
		else if (tick & (state == ST_SHIFT_HI))
			shift_data <= { shift_data[6:0], 1'b0 };
`endif

	// IO control
	always @(posedge clk)
	begin
		// Defaults
		srio_clk_o   <= 1'b0;
		srio_dat_o   <= 1'b0;
		srio_rclk_o  <= 1'b0;
		srio_rclk_oe <= 1'b1;

		// Depends on state
		case (state)
		ST_SHIFT_LO: begin
			srio_dat_o <= shift_data[7];
			srio_clk_o <= 1'b0;
		end

		ST_SHIFT_HI: begin
			srio_dat_o <= shift_data[7];
			srio_clk_o <= 1'b1;
		end

		ST_LATCH: begin
			srio_rclk_o <= 1'b1;
		end

		ST_SENSE: begin
			srio_rclk_o  <= 1'b1;
			srio_rclk_oe <= 1'b0;
		end

		ST_PAUSE: begin
			srio_rclk_o  <= 1'b0;
			srio_rclk_oe <= 1'b0;
		end
		endcase
	end

	// External status
	assign sr_rdy = (state == ST_IDLE);

endmodule // sr_btn_if
