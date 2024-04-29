/*
 * misc.h
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

enum e1_tick_type {
	TICK_TX		= 0,
	TICK_RX_PULSE	= 1,
	TICK_RX_SAMPLE	= 2,
	TICK_RX_ONE	= 3,
};

void     e1_tick_sel(int type);
uint16_t e1_tick_read(int port);

void reboot(int fw);
