/*
 * e1.h
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

void e1_init(bool clk_mode);
void e1_poll(void);
void e1_debug_print(bool data);

volatile uint8_t *e1_data_ptr(int mf, int frame, int ts);
unsigned int e1_data_ofs(int mf, int frame, int ts);

enum e1_platform_led {
	E1P_LED_GREEN		= 0,
	E1P_LED_YELLOW		= 1,
};

enum e1_platform_led_state {
	E1P_LED_ST_OFF		= 0,
	E1P_LED_ST_ON		= 1,
	E1P_LED_ST_BLINK	= 2,
	E1P_LED_ST_BLINK_FAST	= 3
};

/* external function provided by the platform; used by E1 driver to control LEDs */
extern void e1_platform_led_set(uint8_t port, enum e1_platform_led led,
				enum e1_platform_led_state state);
