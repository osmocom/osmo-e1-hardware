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
	struct {
		uint16_t oe_out;
		uint8_t  in;
		uint8_t  _rsvd;
	} gpio;
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
e1_led_run(void)
{
	misc_regs->e1_led |= 0x100;
}

void
e1_led_pause(void)
{
	misc_regs->e1_led &= 0xff;
}

void
e1_led_set(bool enable, uint8_t cfg)
{
	misc_regs->e1_led = (enable ? 0x100 : 0x000) | cfg;
}

void
e1_led_change(enum led_which led, enum led_mode mode)
{
	uint32_t msk =    3 << led;
	uint32_t val = mode << led;

	misc_regs->e1_led = (misc_regs->e1_led & ~msk) | val;
}

uint16_t
e1_tick_read(void)
{
	return misc_regs->e1_tick[0].tx;
}



bool
time_elapsed(uint32_t ref, int tick)
{
	return ((misc_regs->time.now - ref) & 0x7fffffff) >= tick;
}

void
delay(int ms)
{
	uint32_t ref = misc_regs->time.now;
	ms *= SYS_CLK_FREQ / 1000;
	while (!time_elapsed(ref, ms));
}

uint32_t
time_pps_read(void)
{
	return misc_regs->time.pps;
}

uint32_t
time_now_read(void)
{
	return misc_regs->time.now;
}


void
gpio_dir(int n, bool output)
{
	uint16_t mask = 256 << n;

	if (output)
		misc_regs->gpio.oe_out |=  mask;
	else
		misc_regs->gpio.oe_out &= ~mask;
}

void
gpio_out(int n, bool val)
{
	uint16_t mask = 1 << n;

	if (val)
		misc_regs->gpio.oe_out |=  mask;
	else
		misc_regs->gpio.oe_out &= ~mask;
}

bool
gpio_in(int n)
{
	return (misc_regs->gpio.in & (1 << n)) != 0;
}
