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
	union {
		uint32_t e1_tick_sel;
		uint16_t e1_tick[2];
	};
	uint32_t _rsvd1;
	struct {
		uint32_t _rsvd2;
		uint32_t now;
	} time;
} __attribute__((packed,aligned(4)));

static volatile struct misc * const misc_regs = (void*)(MISC_BASE);


void
e1_tick_sel(int type)
{
	misc_regs->e1_tick_sel = (type << 16) | type;
}

uint16_t
e1_tick_read(int port)
{
	return misc_regs->e1_tick[port];
}


void
reboot(int fw)
{
	misc_regs->warmboot = (1 << 2) | (fw << 0);
}
