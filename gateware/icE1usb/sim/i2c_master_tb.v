/*
 * i2c_master_tb.v
 *
 * vim: ts=4 sw=4
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: CERN-OHL-P-2.0
 */

`default_nettype none
`timescale 1ns / 100ps

module i2c_master_tb;

	// Signals
	// -------

	reg rst = 1'b1;
	reg clk = 1'b0;


	// Test bench
	// ----------

	// Setup recording
	initial begin
		$dumpfile("i2c_master_tb.vcd");
		$dumpvars(0,i2c_master_tb);
	end

	// Reset pulse
	initial begin
		# 200 rst = 0;
		# 1000000 $finish;
	end

	// Clocks
	always #10 clk = !clk;


	// DUT
	// ---

	wire scl_oe;
	wire sda_oe;
	wire sda_i;

	reg [7:0] data_in;
	reg ack_in;
	reg [1:0] cmd;
	reg stb;

	wire [7:0] data_out;
	wire ack_out;

	wire ready;

	i2c_master #(
		.DW(3)
	) dut_I (
		.scl_oe(scl_oe),
		.sda_oe(sda_oe),
		.sda_i(sda_i),
		.data_in(data_in),
		.ack_in(ack_in),
		.cmd(cmd),
		.stb(stb),
		.data_out(data_out),
		.ack_out(ack_out),
		.ready(ready),
		.clk(clk),
		.rst(rst)
	);


	// Stimulus
	// --------

	assign sda_i = 1'b0;

	task i2c_cmd;
		input [1:0] a_cmd;
		input [7:0] a_data;
		input a_ack;
		begin
			cmd     <= a_cmd;
			data_in <= a_data;
			ack_in  <= a_ack;
			stb     <= 1'b1;
			@(posedge clk);
			stb     <= 1'b0;
			@(posedge clk);
			wait (ready == 1'b1);
			@(posedge clk);
		end
	endtask

	initial begin
		// Reset
		data_in <= 8'h00;
		ack_in  <= 1'b0;
		cmd     <= 2'b00;
		stb     <= 1'b0;

		wait (rst == 1'b0);
		wait (ready == 1'b1);
		@(posedge clk);

		// Issue commands
		i2c_cmd(2'b00, 8'h00, 1'b0);
		i2c_cmd(2'b10, 8'ha5, 1'b0);
		i2c_cmd(2'b11, 8'h00, 1'b0);
		i2c_cmd(2'b01, 8'h00, 1'b0);

	end

endmodule
