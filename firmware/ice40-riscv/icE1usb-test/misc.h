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


enum led_which {
	LED_CH0_GREEN  = 0,
	LED_CH0_YELLOW = 2,
	LED_CH1_GREEN  = 4,
	LED_CH1_YELLOW = 6,
};

enum led_mode {
	LED_MODE_OFF  = 0,
	LED_MODE_ON   = 1,
	LED_MODE_SLOW = 2,
	LED_MODE_FAST = 3,
};

void e1_led_run(void);
void e1_led_pause(void);
void e1_led_set(bool enable, uint8_t cfg);
void e1_led_change(enum led_which led, enum led_mode mode);

uint16_t e1_tick_read(void);

bool time_elapsed(uint32_t ref, int tick);
void delay(int ms);
uint32_t time_pps_read(void);
uint32_t time_now_read(void);

void gpio_dir(int n, bool output);
void gpio_out(int n, bool val);
bool gpio_in(int n);
