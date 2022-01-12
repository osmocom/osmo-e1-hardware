/*
 * usb_gps.h
 *
 * Copyright (C) 2019-2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

void usb_gps_puts(const char *str);

void usb_gps_poll(void);
void usb_gps_init(void);
