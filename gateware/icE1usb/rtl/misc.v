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
	output wire  [1:0] e1_rx_bias,

	output wire        clk_tune_hi,
	output wire        clk_tune_lo,

	// GPS
	output wire        gps_reset_n,
	input  wire        gps_pps,

	// GPIO
	inout  wire [ 2:0] gpio,

	// E1 led status
	output wire [ 7:0] e1_led_state,
	output wire        e1_led_run,
	input  wire        e1_led_active,

	// Button
	input  wire        btn_val,
	input  wire        btn_stb,

	// Ticks
	input  wire [ 1:0] tick_e1_rx,
	input  wire [ 1:0] tick_e1_tx,
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
	wire        bus_clr;
	reg         bus_we_boot;
	reg         bus_we_gpio;
	reg         bus_we_led;
	reg  [ 1:0] bus_we_pdm_clk;
	reg  [ 1:0] bus_we_pdm_e1;

	// GPIO
	reg  [3:0] gpio_oe;
	reg  [3:0] gpio_out;
	wire [3:0] gpio_in;

	// LED
	reg  [ 8:0] e1_led;

	// PPS sync
	wire gps_pps_iob;
	wire gps_pps_r;

	// Counters
	wire [15:0] cap_e1_rx[0:1];
	wire [15:0] cap_e1_tx[0:1];
	wire [31:0] cap_gps;
	wire [31:0] cnt_time;

	// PDM
	reg  [12:0] pdm_clk[0:1];
	reg  [ 8:0] pdm_e1[0:1];

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
			bus_we_gpio       <= 1'b0;
			bus_we_led        <= 1'b0;
			bus_we_pdm_clk[0] <= 1'b0;
			bus_we_pdm_clk[1] <= 1'b0;
			bus_we_pdm_e1[0]  <= 1'b0;
			bus_we_pdm_e1[1]  <= 1'b0;
		end else begin
			bus_we_boot       <= wb_addr == 4'h0;
			bus_we_gpio       <= wb_addr == 4'h1;
			bus_we_led        <= wb_addr == 4'h2;
			bus_we_pdm_clk[0] <= wb_addr == 4'h8;
			bus_we_pdm_clk[1] <= wb_addr == 4'h9;
			bus_we_pdm_e1[0]  <= wb_addr == 4'ha;
			bus_we_pdm_e1[1]  <= wb_addr == 4'hb;
		end

	// Read mux
	always @(posedge clk)
		if (bus_clr)
			wb_rdata <= 32'h00000000;
		else
			case (wb_addr[3:0])
				4'h1:    wb_rdata <= { 12'h000, gpio_in, 4'h0, gpio_oe, 4'h0, gpio_out };
				4'h2:    wb_rdata <= { 22'h000000, e1_led_active, e1_led };
				4'h4:    wb_rdata <= { cap_e1_tx[0], cap_e1_rx[0] };
				4'h5:    wb_rdata <= { cap_e1_tx[1], cap_e1_rx[1] };
				4'h6:    wb_rdata <= cap_gps;
				4'h7:    wb_rdata <= cnt_time;
`ifdef WITH_PDM_READBACK
				4'h8:    wb_rdata <= { pdm_clk[0][12], 19'h00000, pdm_clk[0][11:0] };
				4'h9:    wb_rdata <= { pdm_clk[1][12], 19'h00000, pdm_clk[1][11:0] };
				4'ha:    wb_rdata <= {  pdm_e1[0][8], 23'h000000,  pdm_e1[0][ 7:0] };
				4'hb:    wb_rdata <= {  pdm_e1[1][8], 23'h000000,  pdm_e1[1][ 7:0] };
`endif
				default: wb_rdata <= 32'hxxxxxxxx;
			endcase


	// GPIO (incl gps reset)
	// ----

	// IOB
	SB_IO #(
		.PIN_TYPE(6'b110100),	// Reg in/out/oe
		.PULLUP(1'b1),
		.IO_STANDARD("SB_LVCMOS")
	) gpio_iob_I[3:0] (
		.PACKAGE_PIN  ({gps_reset_n, gpio}),
		.CLOCK_ENABLE (1'b1),
		.INPUT_CLK    (clk),
		.OUTPUT_CLK   (clk),
		.OUTPUT_ENABLE(gpio_oe),
		.D_OUT_0      (gpio_out),
		.D_IN_0       (gpio_in)
	);

	// Bus
	always @(posedge clk or posedge rst)
		if (rst) begin
			gpio_oe  <= 4'h0;
			gpio_out <= 4'h0;
		end else if (bus_we_gpio) begin
			gpio_oe  <= wb_wdata[11:8];
			gpio_out <= wb_wdata[ 3:0];
		end


	// E1 led status
	// -------------

	always @(posedge clk or posedge rst)
		if (rst)
			e1_led <= 9'h00;
		else if (bus_we_led)
			e1_led <= wb_wdata[8:0];

	assign e1_led_state = e1_led[7:0];
	assign e1_led_run   = e1_led[8];


	// PPS input
	// ---------

	// IO reg
	SB_IO #(
		.PIN_TYPE(6'b000000),	// Reg input, no output
		.PULLUP(1'b0),
		.IO_STANDARD("SB_LVCMOS")
	) btn_iob_I (
		.PACKAGE_PIN(gps_pps),
		.INPUT_CLK  (clk),
		.D_IN_0     (gps_pps_iob)
	);

	// Deglitch
	glitch_filter #(
		.L(2),
		.RST_VAL(1'b0),
		.WITH_SYNCHRONIZER(1)
	) btn_flt_I (
		.in       (gps_pps_iob),
		.val      (),
		.rise     (gps_pps_r),
		.fall     (),
		.clk      (clk),
`ifdef SIM
		.rst      (rst)
`else
		// Don't reset so we let the filter settle before
		// the rest of the logic engages
		.rst      (1'b0)
`endif
	);


	// Counters
	// --------

	// E1 ticks
	capcnt #(
		.W(16)
	) e1_cnt_I[3:0] (
		.cnt_cur (),
		.cnt_cap ({cap_e1_tx[1],  cap_e1_rx[1],  cap_e1_tx[0],  cap_e1_rx[0] }),
		.inc     ({tick_e1_tx[1], tick_e1_rx[1], tick_e1_tx[0], tick_e1_rx[0]}),
		.cap     (tick_usb_sof),
		.clk     (clk),
		.rst     (rst)
	);

	// Time / GPS
	capcnt #(
		.W(32)
	) time_cnt_I (
		.cnt_cur (cnt_time),
		.cnt_cap (cap_gps),
		.inc     (1'b1),
		.cap     (gps_pps_r),
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
		end else begin
			if (bus_we_pdm_clk[0])  pdm_clk[0] <= { wb_wdata[31], wb_wdata[11:0] };
			if (bus_we_pdm_clk[1])  pdm_clk[1] <= { wb_wdata[31], wb_wdata[11:0] };
			if (bus_we_pdm_e1[0])   pdm_e1[0]  <= { wb_wdata[31], wb_wdata[ 7:0] };
			if (bus_we_pdm_e1[1])   pdm_e1[1]  <= { wb_wdata[31], wb_wdata[ 7:0] };
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
	) pdm_e1_I[1:0] (
		.pdm    ({ e1_rx_bias[1],  e1_rx_bias[0]  }),
		.cfg_val({ pdm_e1[1][7:0], pdm_e1[0][7:0] }),
		.cfg_oe ({ pdm_e1[1][8],   pdm_e1[0][8]   }),
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
		.BTN_MODE(0),
		.DFU_MODE(0)
	) dfu_I (
		.boot_sel(boot_sel),
		.boot_now(boot_now),
		.btn_pad (btn_val),
		.btn_tick(btn_stb),
		.btn_val (),
		.rst_req (rst_req),
		.clk     (clk),
		.rst     (rst)
	);

endmodule // misc
