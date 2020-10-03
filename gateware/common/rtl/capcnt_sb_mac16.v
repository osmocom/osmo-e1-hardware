/*
 * capcnt_sb_mac16.v
 *
 * Helper to use the SB_MAC16 as simple capture/counter blocks
 *
 * vim: ts=4 sw=4
 *
 * Copyright (C) 2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: CERN-OHL-P-2.0
 */

`default_nettype none

module capcnt16_sb_mac16 (
	output wire [15:0] cnt_cur,
	output wire [15:0] cnt_cap,
	input  wire        inc,
	input  wire        cap,
	input  wire        clk,
	input  wire        rst
);

	// TOP = Count
	// Top unit configured as 16 bit accumulator with fixed carry=1 input.
	// Most of input/registers are HOLD=1 and forced to reset. The output
	// register HOLD input is used for conditional increment.
	//
	// BOT = Capture
	// Bottom unit is configured for the output register to do fixed loading
	// from D input which is wired to the current counter value. HOLD input
	// used for capture.

	SB_MAC16 #(
		.NEG_TRIGGER             (1'b0),
		.C_REG                   (1'b1),
		.A_REG                   (1'b1),
		.B_REG                   (1'b1),
		.D_REG                   (1'b0),
		.TOP_8x8_MULT_REG        (1'b1),
		.BOT_8x8_MULT_REG        (1'b1),
		.PIPELINE_16x16_MULT_REG1(1'b1),
		.PIPELINE_16x16_MULT_REG2(1'b1),
		.TOPOUTPUT_SELECT        (2'b01),
		.TOPADDSUB_LOWERINPUT    (2'b00),
		.TOPADDSUB_UPPERINPUT    (1'b0),
		.TOPADDSUB_CARRYSELECT   (2'b01),
		.BOTOUTPUT_SELECT        (2'b01),
		.BOTADDSUB_LOWERINPUT    (2'b00),
		.BOTADDSUB_UPPERINPUT    (1'b0),
		.BOTADDSUB_CARRYSELECT   (2'b00),
		.MODE_8x8                (1'b1),
		.A_SIGNED                (1'b0),
		.B_SIGNED                (1'b0)
	) mac_I (
		.CLK        (clk),
		.CE         (1'b1),
		.C          (16'h0000),
		.A          (16'h0000),
		.B          (16'h0000),
		.D          (cnt_cur),
		.AHOLD      (1'b1),
		.BHOLD      (1'b1),
		.CHOLD      (1'b1),
		.DHOLD      (1'b1),
		.IRSTTOP    (1'b1),
		.IRSTBOT    (1'b1),
		.ORSTTOP    (rst),
		.ORSTBOT    (rst),
		.OLOADTOP   (1'b0),
		.OLOADBOT   (1'b1),
		.ADDSUBTOP  (1'b0),
		.ADDSUBBOT  (1'b0),
		.OHOLDTOP   (~inc),
		.OHOLDBOT   (~cap),
		.CI         (),
		.ACCUMCI    (),
		.SIGNEXTIN  (),
		.O          ({cnt_cur, cnt_cap}),
		.CO         (),
		.ACCUMCO    (),
		.SIGNEXTOUT ()
	);

endmodule // capcnt16_sb_mac16


module capcnt32_sb_mac16 (
	output wire [31:0] cnt_cur,
	output wire [31:0] cnt_cap,
	input  wire        inc,
	input  wire        cap,
	input  wire        clk,
	input  wire        rst
);

	// Counting
	// --------
	// Hi/Lo configured as 32 bit accumulator adding a constant carry=1
	// at every cycle. Most register are held in reset and with HOLD=1.
	// HOLD input on Output reg used to implement conditional 'increment'.

	SB_MAC16 #(
		.NEG_TRIGGER             (1'b0),
		.C_REG                   (1'b1),
		.A_REG                   (1'b1),
		.B_REG                   (1'b1),
		.D_REG                   (1'b1),
		.TOP_8x8_MULT_REG        (1'b1),
		.BOT_8x8_MULT_REG        (1'b1),
		.PIPELINE_16x16_MULT_REG1(1'b1),
		.PIPELINE_16x16_MULT_REG2(1'b1),
		.TOPOUTPUT_SELECT        (2'b01),
		.TOPADDSUB_LOWERINPUT    (2'b00),
		.TOPADDSUB_UPPERINPUT    (1'b0),
		.TOPADDSUB_CARRYSELECT   (2'b10),
		.BOTOUTPUT_SELECT        (2'b01),
		.BOTADDSUB_LOWERINPUT    (2'b00),
		.BOTADDSUB_UPPERINPUT    (1'b0),
		.BOTADDSUB_CARRYSELECT   (2'b01),
		.MODE_8x8                (1'b1),
		.A_SIGNED                (1'b0),
		.B_SIGNED                (1'b0)
	) cnt_mac_I (
		.CLK        (clk),
		.CE         (1'b1),
		.C          (16'h0000),
		.A          (16'h0000),
		.B          (16'h0000),
		.D          (16'h0000),
		.AHOLD      (1'b1),
		.BHOLD      (1'b1),
		.CHOLD      (1'b1),
		.DHOLD      (1'b1),
		.IRSTTOP    (1'b1),
		.IRSTBOT    (1'b1),
		.ORSTTOP    (rst),
		.ORSTBOT    (rst),
		.OLOADTOP   (1'b0),
		.OLOADBOT   (1'b0),
		.ADDSUBTOP  (1'b0),
		.ADDSUBBOT  (1'b0),
		.OHOLDTOP   (~inc),
		.OHOLDBOT   (~inc),
		.CI         (),
		.ACCUMCI    (),
		.SIGNEXTIN  (),
		.O          (cnt_cur),
		.CO         (),
		.ACCUMCO    (),
		.SIGNEXTOUT ()
	);


	// Capture
	// -------
	// Output register is used to capture the value.
	// It's loaded from {C,D} and using HOLD on the output
	// register to implement capture trigger.

	SB_MAC16 #(
		.NEG_TRIGGER             (1'b0),
		.C_REG                   (1'b0),
		.A_REG                   (1'b1),
		.B_REG                   (1'b1),
		.D_REG                   (1'b0),
		.TOP_8x8_MULT_REG        (1'b1),
		.BOT_8x8_MULT_REG        (1'b1),
		.PIPELINE_16x16_MULT_REG1(1'b1),
		.PIPELINE_16x16_MULT_REG2(1'b1),
		.TOPOUTPUT_SELECT        (2'b01),
		.TOPADDSUB_LOWERINPUT    (2'b00),
		.TOPADDSUB_UPPERINPUT    (1'b0),
		.TOPADDSUB_CARRYSELECT   (2'b00),
		.BOTOUTPUT_SELECT        (2'b01),
		.BOTADDSUB_LOWERINPUT    (2'b00),
		.BOTADDSUB_UPPERINPUT    (1'b0),
		.BOTADDSUB_CARRYSELECT   (2'b00),
		.MODE_8x8                (1'b1),
		.A_SIGNED                (1'b0),
		.B_SIGNED                (1'b0)
	) cap_mac_I (
		.CLK        (clk),
		.CE         (1'b1),
		.C          (cnt_cur[31:16]),
		.A          (16'h0000),
		.B          (16'h0000),
		.D          (cnt_cur[15:0]),
		.AHOLD      (1'b1),
		.BHOLD      (1'b1),
		.CHOLD      (1'b1),
		.DHOLD      (1'b1),
		.IRSTTOP    (1'b1),
		.IRSTBOT    (1'b1),
		.ORSTTOP    (rst),
		.ORSTBOT    (rst),
		.OLOADTOP   (1'b1),
		.OLOADBOT   (1'b1),
		.ADDSUBTOP  (1'b0),
		.ADDSUBBOT  (1'b0),
		.OHOLDTOP   (~cap),
		.OHOLDBOT   (~cap),
		.CI         (),
		.ACCUMCI    (),
		.SIGNEXTIN  (),
		.O          (cnt_cap),
		.CO         (),
		.ACCUMCO    (),
		.SIGNEXTOUT ()
	);

endmodule // capcnt32_sb_mac16
