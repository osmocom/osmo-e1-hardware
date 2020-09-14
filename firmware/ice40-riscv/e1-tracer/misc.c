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
		uint16_t tx;
		uint16_t rx;
	} e1_tick[2];
	uint32_t _rsvd1;
	uint32_t time;
} __attribute__((packed,aligned(4)));

static volatile struct misc * const misc_regs = (void*)(MISC_BASE);


void
e1_tick_read(uint16_t *ticks)
{
	uint32_t v = misc_regs->e1_tick;
	ticks[0] = (v      ) & 0xffff;
	ticks[1] = (v >> 16) & 0xffff;
}
