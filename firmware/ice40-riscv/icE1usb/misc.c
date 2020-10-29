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

uint16_t
e1_tick_read(void)
{
	return misc_regs->e1_tick[0].tx;
}

void
reboot(int fw)
{
	misc_regs->warmboot = (1 << 2) | (fw << 0);
}
