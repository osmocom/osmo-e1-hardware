/*
 * gpsdo.c
 *
 * Copyright (C) 2019-2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include <string.h>

#include "console.h"
#include "gps.h"
#include "gpsdo.h"
#include "misc.h"

#include "ice1usb_proto.h"


struct {
	/* Current tuning */
	struct {
		uint16_t coarse;
		uint16_t fine;
	} tune;

	/* Measurements */
	struct {
		uint32_t tick_prev;	/* Previous tick value */
		uint32_t last;		/* Last measurement */
		int      skip;		/* Number of measurement left to discard */
		int      invalid;	/* Number of consecutive invalid measurements */
	} meas;

	/* FSM */
	enum {
		STATE_DISABLED,		/* Forced to manual */
		STATE_HOLD_OVER,	/* GPS invalid data, hold */
		STATE_TUNE_COARSE,	/* Adjust coarse tuning until we're +- 3 Hz */
		STATE_TUNE_FINE,	/* Fine tracking */
	} state;

	/* Coarse tuning */
	struct {
		int vctxo_sensitivity;
		int step;
	} coarse;

	/* Fine tracking */
	struct {
		int acc;
		int div;
	} fine;

} g_gpsdo;


/* VCXTO sensitivity vs 'coarse' count for fast initial acquisition */
	/*
	 * Note that the spec if often a guaranteed minimum range and goes
	 * from ~0.1V to 3.2V instead of 0-3.3V so actual sensitivity is
	 * higher than the "theoritical value". We boost it by ~ 10% here.
	 */
static const int
vctxo_sensitivity[] = {
	/* +-  50 ppm pull range => ~ 0.75 Hz / hi-count (set to 0.85) */
	[VCTXO_TAITIEN_VT40]     = 300,

	/* +- 100 ppm pull range => ~ 1.50 Hz / hi-count (set to 1.6) */
	[VCTXO_SITIME_SIT3808_E] = 160,
};

/* Tuning params */
#define TARGET		30720000
#define MAX_DEV_VALID	1000
#define MAX_DEV_FINE	3
#define MAX_INVALID	5


static void _gpsdo_coarse_start(void);


void
gpsdo_get_status(struct e1usb_gpsdo_status *status)
{
	const uint8_t state_map[] = {
		[STATE_DISABLED]    = ICE1USB_GPSDO_STATE_DISABLED,
		[STATE_HOLD_OVER]   = ICE1USB_GPSDO_STATE_HOLD_OVER,
		[STATE_TUNE_COARSE] = ICE1USB_GPSDO_STATE_TUNE_COARSE,
		[STATE_TUNE_FINE]   = ICE1USB_GPSDO_STATE_TUNE_FINE,
	};
	const uint8_t ant_map[] = {
		[ANT_UNKNOWN] = ICE1USB_GPSDO_ANT_UNKNOWN,
		[ANT_OK]      = ICE1USB_GPSDO_ANT_OK,
		[ANT_OPEN]    = ICE1USB_GPSDO_ANT_OPEN,
		[ANT_SHORT]   = ICE1USB_GPSDO_ANT_SHORT,
	};

	status->state          = state_map[g_gpsdo.state];
	status->antenna_state  = ant_map[gps_antenna_status()];
	status->valid_fix      = gps_has_valid_fix();
	status->mode           = (g_gpsdo.state == STATE_DISABLED) ? ICE1USB_GPSDO_MODE_DISABLED : ICE1USB_GPSDO_MODE_AUTO;
	status->tune.coarse    = g_gpsdo.tune.coarse;
	status->tune.fine      = g_gpsdo.tune.fine;
	status->freq_est       = g_gpsdo.meas.last;
}

void
gpsdo_enable(bool enable)
{
	if (!enable)
		g_gpsdo.state = STATE_DISABLED;
	else if (g_gpsdo.state == STATE_DISABLED)
		g_gpsdo.state = STATE_HOLD_OVER;
}

bool
gpsdo_enabled(void)
{
	return g_gpsdo.state != STATE_DISABLED;
}

void
gpsdo_set_tune(uint16_t  coarse, uint16_t  fine)
{
	/* Set value */
	g_gpsdo.tune.coarse = coarse;
	g_gpsdo.tune.fine   = fine;

	pdm_set(PDM_CLK_HI, true, g_gpsdo.tune.coarse, false);
	pdm_set(PDM_CLK_LO, true, g_gpsdo.tune.fine,   false);

	/* If we were in 'fine' mode, reset to coarse */
	if (g_gpsdo.state == STATE_TUNE_FINE)
		_gpsdo_coarse_start();
}

void
gpsdo_get_tune(uint16_t *coarse, uint16_t *fine)
{
	*coarse = g_gpsdo.tune.coarse;
	*fine   = g_gpsdo.tune.fine;
}


static void
_gpsdo_coarse_start(void)
{
	/* Set the state */
	g_gpsdo.state = STATE_TUNE_COARSE;

	/* Skip a few measurements to be safe */
	g_gpsdo.meas.skip = 3;

	/* Reset coarse tuning state */
	g_gpsdo.coarse.step = 0;

	/* Put the fine adjust back in the middle point */
	g_gpsdo.tune.coarse += ((int)g_gpsdo.tune.fine - 2048) >> 6;
	g_gpsdo.tune.fine    = 2048;

	pdm_set(PDM_CLK_HI, true, g_gpsdo.tune.coarse, false);
	pdm_set(PDM_CLK_LO, true, g_gpsdo.tune.fine,   false);
}

static void
_gpsdo_fine_start(void)
{
	/* Set the state */
	g_gpsdo.state = STATE_TUNE_FINE;

	/* Reset the long term error tracking */
	g_gpsdo.fine.acc = 0;
	g_gpsdo.fine.div = 0;
}

static void
_gpsdo_coarse_tune(uint32_t tick_diff)
{
	int freq_diff = (int)tick_diff - TARGET;

	/* Is the measurement good enough to switch to fine ? */
	if ((freq_diff > -MAX_DEV_FINE) && (freq_diff < MAX_DEV_FINE)) {
		_gpsdo_fine_start();
		return;
	}

	/* Estimate correction and apply it */
	g_gpsdo.tune.coarse -= (freq_diff * g_gpsdo.coarse.vctxo_sensitivity) >> 8;
	pdm_set(PDM_CLK_HI, true, g_gpsdo.tune.coarse, false);

	/* Skip next measurement */
	g_gpsdo.meas.skip = 1;

	/* Debug */
#ifdef GPSDO_DEBUG
	printf("[+] GPSDO Coarse: err=%d tune=%d\n", freq_diff, g_gpsdo.tune.coarse);
#endif
}

static void
_gpsdo_fine_track(uint32_t tick_diff)
{
	int freq_diff = (int)tick_diff - TARGET;
	uint16_t tune;

	/* Did we deviate too much ? */
	if ((freq_diff < -2*MAX_DEV_FINE) && (freq_diff > 2*MAX_DEV_FINE)) {
		_gpsdo_coarse_start();
		return;
	}

	/* Accumulate long term error */
	g_gpsdo.fine.acc += freq_diff;

	/* Update fine tuning value */
	if (((g_gpsdo.fine.acc >= 0) && (freq_diff > 0)) ||
	    ((g_gpsdo.fine.acc <= 0) && (freq_diff < 0)))
	{
		    g_gpsdo.tune.fine -= freq_diff;
	}

	if (g_gpsdo.fine.acc) {
		if (++g_gpsdo.fine.div > 10) {
			g_gpsdo.fine.div = 0;
			if (g_gpsdo.fine.acc > 0)
				g_gpsdo.tune.fine--;
			else if (g_gpsdo.fine.acc < 0)
				g_gpsdo.tune.fine++;
		}
	} else {
		g_gpsdo.fine.div = 0;
	}

	/* Apply value with a bias from long term accumulator */
	tune = g_gpsdo.tune.fine - (g_gpsdo.fine.acc / 2);
	pdm_set(PDM_CLK_LO, true, tune, false);

	/* Debug */
#ifdef GPSDO_DEBUG
	printf("[+] GPSDO Fine: err=%d acc=%d tune=%d\n", freq_diff, g_gpsdo.fine.acc, g_gpsdo.tune.fine);
#endif
}


void
gpsdo_poll(void)
{
	uint32_t tick_now, tick_diff;
	bool valid;

	/* Get current tick and check if there was a PPS and estimate frequency */
	tick_now = time_pps_read();

	if (tick_now == g_gpsdo.meas.tick_prev)
		return;

	g_gpsdo.meas.last = tick_diff = tick_now - g_gpsdo.meas.tick_prev;
	g_gpsdo.meas.tick_prev = tick_now;

	/* If we're currently discarding samples, skip it */
	if (g_gpsdo.meas.skip) {
		g_gpsdo.meas.skip--;
		return;
	}

	/* If we're disabled, nothing else to do */
	if (g_gpsdo.state == STATE_DISABLED)
		return;

	/* Check GPS state */
	if (!gps_has_valid_fix()) {
		/* No GPS fix, go to hold-over */
		g_gpsdo.state = STATE_HOLD_OVER;
		return;
	}

	/* Check measurement plausibility */
	valid = (tick_diff > (TARGET - MAX_DEV_VALID)) && (tick_diff < (TARGET + MAX_DEV_VALID));

	if (valid) {
		/* If we're in hold-over, switch to active */
		if (g_gpsdo.state == STATE_HOLD_OVER) {
			_gpsdo_coarse_start();
			return;
		}
	} else {
		/* Count invalid measurements and if too many of
		 * them, we go back to hold-over */
		if (++g_gpsdo.meas.invalid >= MAX_INVALID) {
			g_gpsdo.state = STATE_HOLD_OVER;
			return;
		}
	}

	g_gpsdo.meas.invalid = 0;

	/* If we reach here, we have a valid fix, valid measurement and
	 * we're not in hold-over or disabled. Feed the correct loop */
	if (g_gpsdo.state == STATE_TUNE_COARSE)
		_gpsdo_coarse_tune(tick_diff);
	else if (g_gpsdo.state == STATE_TUNE_FINE)
		_gpsdo_fine_track(tick_diff);
}


void
gpsdo_init(enum gpsdo_vctxo_model vctxo)
{
	/* State */
	memset(&g_gpsdo, 0x00, sizeof(g_gpsdo));

	/* Default tune to middle range */
	g_gpsdo.tune.coarse = 2048;
	g_gpsdo.tune.fine   = 2048;

	pdm_set(PDM_CLK_HI, true, g_gpsdo.tune.coarse, false);
	pdm_set(PDM_CLK_LO, true, g_gpsdo.tune.fine,   false);

	/* Initial state and config */
	g_gpsdo.state = STATE_HOLD_OVER;
	g_gpsdo.coarse.vctxo_sensitivity = vctxo_sensitivity[vctxo];
}
