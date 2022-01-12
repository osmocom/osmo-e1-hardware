/*
 * gpsdo.h
 *
 * Copyright (C) 2019-2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

enum gpsdo_vctxo_model {
	VCTXO_TAITIEN_VT40 = 0,		/* VTEUALJANF-30.720000 */
	VCTXO_SITIME_SIT3808_E = 1,	/* SIT3808AI-D2-33EE-30.720000T */
};

struct e1usb_gpsdo_status;


void gpsdo_get_status(struct e1usb_gpsdo_status *status);

void gpsdo_enable(bool enable);
bool gpsdo_enabled(void);

void gpsdo_set_tune(uint16_t  coarse, uint16_t  fine);
void gpsdo_get_tune(uint16_t *coarse, uint16_t *fine);

void gpsdo_poll(void);
void gpsdo_init(enum gpsdo_vctxo_model vctxo);
