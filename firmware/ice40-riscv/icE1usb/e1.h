/*
 * e1.h
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

void e1_init(uint16_t rx_cr, uint16_t tx_cr);
void e1_poll(void);
void e1_debug_print(bool data);

void e1_tx_config(uint16_t cr);
void e1_rx_config(uint16_t cr);

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
