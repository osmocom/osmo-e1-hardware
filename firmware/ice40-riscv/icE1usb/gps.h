/*
 * gps.h
 *
 * Copyright (C) 2019-2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>

enum gps_antenna_state {
	ANT_UNKNOWN = 0,
	ANT_OK,
	ANT_OPEN,
	ANT_SHORT
};

bool gps_has_valid_fix(void);
enum gps_antenna_state gps_antenna_status(void);

void gps_poll(void);
void gps_init(void);
