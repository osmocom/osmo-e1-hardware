/*
 * misc.h
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

enum pdm_chan {
	PDM_CLK_LO	= 0,
	PDM_CLK_HI	= 1,

	/* icE1usb */
	PDM_E1_RX0	= 2,
	PDM_E1_RX1	= 3,

	/* icE1usb-proto */
	PDM_E1_N	= 2,
	PDM_E1_P	= 3,
	PDM_E1_CT	= 4,
};

void pdm_set(int chan, bool enable, unsigned value, bool normalize);

void e1_led_set(bool enable, uint8_t cfg);
uint16_t e1_tick_read(void);

bool time_elapsed(uint32_t ref, int tick);
void delay(int ms);
uint32_t time_pps_read(void);
uint32_t time_now_read(void);

void gpio_dir(int n, bool output);
void gpio_out(int n, bool val);
bool gpio_in(int n);

void reboot(int fw);
