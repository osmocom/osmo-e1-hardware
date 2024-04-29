/*
 * top.v
 *
 * vim: ts=4 sw=4
 *
 * Top-level for the icE1usb production boards
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: CERN-OHL-S-2.0
 */

`default_nettype none

// `define WITH_SINGLE_CHANNEL
// `define WITH_SB_I2C
`define WITH_CUSTOM_I2C

module top (
	// E1 PHY
	input  wire e1A_rx_hi_p,
//	input  wire e1A_rx_hi_n,
	input  wire e1A_rx_lo_p,
//	input  wire e1A_rx_lo_n,
	output wire e1A_tx_hi,
	output wire e1A_tx_lo,

	input  wire e1B_rx_hi_p,
//	input  wire e1B_rx_hi_n,
	input  wire e1B_rx_lo_p,
//	input  wire e1B_rx_lo_n,
	output wire e1B_tx_hi,
	output wire e1B_tx_lo,

	output wire [1:0] e1_rx_bias,

	// USB
	inout  wire usb_dp,
	inout  wire usb_dn,
	output wire usb_pu,

	// Flash
	inout  wire flash_mosi,
	inout  wire flash_miso,
	inout  wire flash_clk,
	inout  wire flash_cs_n,

	// LED Shift register + Button input
	inout  wire e1_led_rclk,

	// GPS
	output wire gps_reset_n,
	input  wire gps_rx,
	output wire gps_tx,
	input  wire gps_pps,

	// I2C
	inout  wire i2c_sda,
	inout  wire i2c_scl,

	// GPIOs
	inout  wire [2:0] gpio,

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

	localparam integer WB_N = 3;

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
	wire [7:0] tick_e1;
	wire       tick_usb_sof;

	// I2C
	wire i2c_scl_oe;
	wire i2c_scl_i;
	wire i2c_sda_oe;
	wire i2c_sda_i;

	// Led & Button
	wire [7:0] e1_led_state;
	wire       e1_led_run;
	wire       e1_led_active;

	wire       spi_req;
	wire       spi_gnt;

	wire [7:0] sr_val;
	wire       sr_go;
	wire       sr_rdy;

	wire       btn_val;
	wire       btn_stb;

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
`ifdef WITH_SINGLE_CHANNEL
		.E1_UNIT_HAS_RX(2'b01),
		.E1_UNIT_HAS_TX(2'b01),
`else
		.E1_UNIT_HAS_RX(2'b11),
		.E1_UNIT_HAS_TX(2'b11),
`endif
		.E1_LIU(0)
	) soc_I (
		.e1_rx_hi_p   ({e1B_rx_hi_p, e1A_rx_hi_p}),
//		.e1_rx_hi_n   ({e1B_rx_hi_n, e1A_rx_hi_n}),
		.e1_rx_lo_p   ({e1B_rx_lo_p, e1A_rx_lo_p}),
//		.e1_rx_lo_n   ({e1B_rx_lo_n, e1A_rx_lo_n}),
		.e1_tx_hi     ({e1B_tx_hi,   e1A_tx_hi  }),
		.e1_tx_lo     ({e1B_tx_lo,   e1A_tx_lo  }),
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
		.tick_e1      (tick_e1),
		.tick_usb_sof (tick_usb_sof),
		.clk_sys      (clk_sys),
		.rst_sys      (rst_sys),
		.clk_48m      (clk_48m),
		.rst_48m      (rst_48m)
	);

	// WB read data flattening
	for (i=0; i<WB_N; i=i+1)
		assign wb_rdata_flat[i*32+:32] = wb_rdata[i];


	// Dummy channel
	// -------------

`ifdef WITH_SINGLE_CHANNEL
	wire [1:0] e1_dummy;

	SB_IO #(
		.PIN_TYPE(6'b000000),
		.PULLUP(1'b0),
		.NEG_TRIGGER(1'b0),
		.IO_STANDARD("SB_LVDS_INPUT")
	) e1_dummy_rx_I[1:0] (
		.PACKAGE_PIN({e1B_rx_hi_p, e1B_rx_lo_p}),
		.LATCH_INPUT_VALUE(1'b0),
		.CLOCK_ENABLE(1'b1),
		.INPUT_CLK(clk_sys),
		.OUTPUT_CLK(1'b0),
		.OUTPUT_ENABLE(1'b0),
		.D_OUT_0(1'b0),
		.D_OUT_1(1'b0),
		.D_IN_0(e1_dummy),
		.D_IN_1()
	);

	SB_IO #(
		.PIN_TYPE(6'b010100),
		.PULLUP(1'b0),
		.NEG_TRIGGER(1'b0),
		.IO_STANDARD("SB_LVCMOS")
	) e1_dummy_tx_I[1:0] (
		.PACKAGE_PIN({e1B_tx_hi, e1B_tx_lo}),
		.LATCH_INPUT_VALUE(1'b0),
		.CLOCK_ENABLE(1'b1),
		.INPUT_CLK(1'b0),
		.OUTPUT_CLK(clk_sys),
		.OUTPUT_ENABLE(1'b0),
		.D_OUT_0(e1_dummy),
		.D_OUT_1(1'b0),
		.D_IN_0(),
		.D_IN_1()
	);
`endif


	// Misc [0]
	// ----

	misc misc_I (
		.e1_rx_bias    (e1_rx_bias),
		.clk_tune_hi   (clk_tune_hi),
		.clk_tune_lo   (clk_tune_lo),
		.gps_reset_n   (gps_reset_n),
		.gps_pps       (gps_pps),
		.gpio          (gpio),
		.e1_led_state  (e1_led_state),
		.e1_led_run    (e1_led_run),
		.e1_led_active (e1_led_active),
		.btn_val       (btn_val),
		.btn_stb       (btn_stb),
		.tick_e1       (tick_e1),
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


	// GPS UART [1]
	// --------

	uart_wb #(
		.DIV_WIDTH(12),
		.DW(32)
	) gps_uart_I (
		.uart_tx  (gps_tx),
		.uart_rx  (gps_rx),
		.wb_addr  (wb_addr[1:0]),
		.wb_rdata (wb_rdata[1]),
		.wb_wdata (wb_wdata),
		.wb_we    (wb_we),
		.wb_cyc   (wb_cyc[1]),
		.wb_ack   (wb_ack[1]),
		.clk      (clk_sys),
		.rst      (rst_sys)
	);


	// I2C [2]
	// ---

`ifdef WITH_SB_I2C

	// Hard-IP
	ice40_i2c_wb #(
		.WITH_IOB(1),
		.UNIT(0)
	) i2c_I (
		.i2c_scl  (i2c_scl),
		.i2c_sda  (i2c_sda),
		.wb_addr  (wb_addr[3:0]),
		.wb_rdata (wb_rdata[2]),
		.wb_wdata (wb_wdata),
		.wb_we    (wb_we),
		.wb_cyc   (wb_cyc[2]),
		.wb_ack   (wb_ack[2]),
		.clk      (clk_sys),
		.rst      (rst_sys)
	);

`elsif WITH_CUSTOM_I2C

	// Controller
	i2c_master_wb #(
		.DW(4),
		.FIFO_DEPTH(0)
	) i2c_I (
		.scl_oe  (i2c_scl_oe),
		.scl_i   (i2c_scl_i),
		.sda_oe  (i2c_sda_oe),
		.sda_i   (i2c_sda_i),
		.wb_rdata(wb_rdata[2]),
		.wb_wdata(wb_wdata),
		.wb_we   (wb_we),
		.wb_cyc  (wb_cyc[2]),
		.wb_ack  (wb_ack[2]),
		.clk     (clk_sys),
		.rst     (rst_sys)
	);

	// IOBs
	SB_IO #(
		.PIN_TYPE(6'b110100),
		.PULLUP(1'b1),
		.IO_STANDARD("SB_LVCMOS")
	) i2c_iob_I[1:0] (
		.PACKAGE_PIN  ({i2c_scl, i2c_sda}),
		.INPUT_CLK    (clk_sys),
		.OUTPUT_CLK   (clk_sys),
		.OUTPUT_ENABLE({i2c_scl_oe, i2c_sda_oe}),
		.D_OUT_0      (1'b0),
		.D_IN_0       ({i2c_scl_i, i2c_sda_i})
	);

`else

	// Dummy
	assign wb_ack[2] = wb_cyc[2];
	assign wb_rdata[2] = 32'h00000000;

`endif


	// E1 LEDs & Button
	// ----------------

	// Blink pattern generator
	led_blinker blinker_I (
		.led_state(e1_led_state),
		.sr_val   (sr_val),
		.sr_go    (sr_go),
		.sr_rdy   (sr_rdy),
		.clk      (clk_sys),
		.rst      (rst_sys)
	);

	// Interface
	sr_btn_if #(
		.TICK_LOG2_DIV(3)
	) spi_mux_I (
		.flash_mosi  (flash_mosi),
		.flash_miso  (flash_miso),
		.flash_clk   (flash_clk),
		.flash_cs_n  (flash_cs_n),
		.e1_led_rclk (e1_led_rclk),
		.spi_mosi_i  (flash_mosi_i),
		.spi_mosi_o  (flash_mosi_o),
		.spi_mosi_oe (flash_mosi_oe),
		.spi_miso_i  (flash_miso_i),
		.spi_miso_o  (flash_miso_o),
		.spi_miso_oe (flash_miso_oe),
		.spi_clk_i   (flash_clk_i),
		.spi_clk_o   (flash_clk_o),
		.spi_clk_oe  (flash_clk_oe),
		.spi_csn_o   (flash_csn_o),
		.spi_csn_oe  (1'b1),
		.spi_req     (spi_req),
		.spi_gnt     (spi_gnt),
		.sr_val      (sr_val),
		.sr_go       (sr_go),
		.sr_rdy      (sr_rdy),
		.btn_val     (btn_val),
		.btn_stb     (btn_stb),
		.clk         (clk_sys),
		.rst         (rst_sys)
	);

	assign spi_req = ~e1_led_run;
	assign e1_led_active = ~spi_gnt;


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
