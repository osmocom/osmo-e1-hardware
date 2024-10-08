/*
 * e1.h
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once


/* control */

void e1_init(int port, uint16_t rx_cr, uint16_t tx_cr);
void e1_start(int port);
void e1_stop(int port);

void e1_poll(int port);
void e1_linemon_update(void);
void e1_debug_print(int port, bool data);

void e1_rx_config(int port, uint16_t cr);
void e1_tx_config(int port, uint16_t cr);


/* data flow */

unsigned int e1_rx_need_data(int port, unsigned int usb_addr, unsigned int max_len, unsigned int *pos);
unsigned int e1_tx_feed_data(int port, unsigned int usb_addr, unsigned int len);
unsigned int e1_rx_level(int port);
unsigned int e1_tx_level(int port);


/* error reporting */

#define E1_ERR_F_ALIGN_ERR	0x01
#define E1_ERR_F_LOS		0x02
#define E1_ERR_F_RAI		0x04
#define E1_ERR_F_AIS		0x08

struct e1_error_count {
	uint16_t crc;
	uint16_t align;
	uint16_t ovfl;
	uint16_t unfl;
	uint8_t flags;
};

const struct e1_error_count *e1_get_error_count(int port);


/* external function provided by the platform; used by E1 driver to control LEDs */

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

extern void e1_platform_led_set(int port, enum e1_platform_led led,
				enum e1_platform_led_state state);
