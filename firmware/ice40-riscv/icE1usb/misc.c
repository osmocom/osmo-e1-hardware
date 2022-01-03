/*
 * misc.c
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "misc.h"
#include "e1.h"


struct misc {
	uint32_t warmboot;
	uint32_t gpio;
	uint32_t e1_led;
	uint32_t _rsvd;
	struct {
		uint16_t rx;
		uint16_t tx;
	} e1_tick[2];
	struct {
		uint32_t pps;
		uint32_t now;
	} time;
	uint32_t pdm[8];
} __attribute__((packed,aligned(4)));

static volatile struct misc * const misc_regs = (void*)(MISC_BASE);


static const int pdm_bits[5] = { 12, 12, 8, 8, 8 };


void
pdm_set(int chan, bool enable, unsigned value, bool normalize)
{
	if (normalize)
		value >>= (16 - pdm_bits[chan]);
	if (enable)
		value |= 0x80000000;
	misc_regs->pdm[chan] = value;
}


void
e1_led_set(bool enable, uint8_t cfg)
{
	misc_regs->e1_led = (enable ? 0x100 : 0x000) | cfg;
}

void
e1_platform_led_set(int port, enum e1_platform_led led,
		    enum e1_platform_led_state state)
{
	uint32_t tmp;
	unsigned int shift;

	if (port >= 2)
		return;

	shift = 4*port + 2*led;

	tmp = misc_regs->e1_led;
	tmp &= ~(3 << shift);
	tmp |= 0x100 | ((state & 3) << shift);
	misc_regs->e1_led = tmp;
}

uint16_t
e1_tick_read(int port)
{
	return misc_regs->e1_tick[port].tx;
}

void
reboot(int fw)
{
	misc_regs->warmboot = (1 << 2) | (fw << 0);
}
