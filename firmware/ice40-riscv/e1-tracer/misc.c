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
	uint32_t _rsvd0[3];;
	struct {
		uint16_t rx;
		uint16_t _rsvd1;
	} e1_tick[2];
	struct {
		uint32_t _rsvd2;
		uint32_t now;
	} time;
} __attribute__((packed,aligned(4)));

static volatile struct misc * const misc_regs = (void*)(MISC_BASE);


void
e1_tick_read(uint16_t *ticks)
{
	ticks[0] = misc_regs->e1_tick[0].rx;
	ticks[1] = misc_regs->e1_tick[1].rx;
}


void
reboot(int fw)
{
	misc_regs->warmboot = (1 << 2) | (fw << 0);
}
