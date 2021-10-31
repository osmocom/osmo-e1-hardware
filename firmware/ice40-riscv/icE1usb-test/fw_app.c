/*
 * fw_app.c
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <no2usb/usb.h>
#include <no2usb/usb_dfu_rt.h>

#include "config.h"

#include "console.h"
#include "e1.h"
#include "e1_hw.h"
#include "led.h"
#include "misc.h"
#include "mini-printf.h"
#include "spi.h"
#include "utils.h"



// ---------------------------------------------------------------------------
// GPS
// ---------------------------------------------------------------------------

struct wb_uart {
	uint32_t data;
	uint32_t clkdiv;
} __attribute__((packed,aligned(4)));

static volatile struct wb_uart * const gps_uart = (void*)(GPS_UART_BASE);

static struct {
	enum {
		GS_WAIT   = 0,
		GS_READ   = 1,
		GS_CK_HI  = 2,
		GS_CK_LO  = 3,
		GS_END_CR = 4,
		GS_END_LF = 5,
	} state;

	int     len;
	uint8_t cksum;
	char    buf[80];
} gps;


static void
_gps_empty(bool wait_eol)
{
	bool eol = false;

	uint32_t c;
	while (1) {
		c = gps_uart->data;
		if (c & 0x80000000) {
			if (!wait_eol || eol)
				break;
		} else {
			eol = (c == '\n');
		}
	}
}

void
gps_send(const char *s)
{
	char cksum = 0;

	/* Start sentence */
	gps_uart->data = '$';

	/* Send payload */
	while (*s)
		cksum ^= (gps_uart->data = *s++);

	/* Send checksum */
	gps_uart->data = '*';

	s = hexstr(&cksum, 1, false);
	gps_uart->data = *s++;
	gps_uart->data = *s++;

	gps_uart->data = '\r';
	gps_uart->data = '\n';
}

const char *
gps_poll(void)
{
	uint32_t c;
	while (1) {
		/* Get next char */
		c = gps_uart->data;
		if (c & 0x80000000)
			break;

		/* State */
		if (c == '$') {
			/* '$' always triggers reset */
			gps.state = GS_READ;
			gps.len   = 0;
			gps.cksum = 0;
		} else {
			switch (gps.state) {
			case GS_READ:
				if (c == '*') {
					gps.state = GS_CK_HI;
				} else if (gps.len == sizeof(gps.buf)) {
					gps.state = GS_WAIT;
				} else {
					gps.buf[gps.len++] = c;
					gps.cksum ^= c;
				}
				break;
			case GS_CK_HI:
				gps.cksum ^= hexval(c) << 4;
				gps.state = GS_CK_LO;
				break;
			case GS_CK_LO:
				gps.cksum ^= hexval(c);
				gps.state = GS_END_CR;
				break;
			case GS_END_CR:
				gps.state = (c == '\r') ? GS_END_LF : GS_WAIT;
				break;
			case GS_END_LF:
				gps.state = GS_WAIT;
				gps.buf[gps.len] = 0x00;
				if (c == '\n')
					return gps.buf;
				break;
			default:
				gps.state = GS_WAIT;
				break;
			}
		}
	}
	return NULL;
}

#define UART_DIV(baud) ((SYS_CLK_FREQ+(baud)/2)/(baud)-2)

void
gps_init(void)
{
	int i;

	/* State init */
	memset(&gps, 0x00, sizeof(gps));

	/* Configure reset gpio */
	gpio_out(3, false);
	gpio_dir(3, true);

	/* Attempt reset sequence at 9600 baud and then 115200 baud */
	for (i=0; i<2; i++)
	{
		uint32_t start_time = time_now_read();
		bool init_ok;

		/* Assert reset */
		gpio_out(3, false);

		/* Configure uart and empty buffer */
		gps_uart->clkdiv = i ? UART_DIV(115200) : UART_DIV(9600);
		_gps_empty(false);

		/* Wait 100 ms */
		delay(100);

		/* Release reset line */
		gpio_out(3, true);

		/* Wait for first line of output as sign it's ready, timeout after 1s */
		while (!time_elapsed(start_time, SYS_CLK_FREQ))
			if ((init_ok = (gps_poll() != NULL)))
				break;

		if (init_ok) {
			printf("[+] GPS ok at %d baud\n", i ? 115200 : 9600);
			break;
		}
	}

	/* Failed ? */
	if (i == 2) {
		printf("[!] GPS init failed\n");
		return;
	}

#if 0
	/* If success was at 9600 baud, need to speed up */
	if (i == 0) {
		/* Configure GPS to use serial at 115200 baud */
		gps_send("PCAS01,5");

		/* Add dummy byte which will be mangled during baudrate switch ... */
		gps_uart->data = 0x00;
		while (!(gps_uart->clkdiv & (1<<29)));

		/* Set uart to 115200 and empty uart buffer, line aligned */
		gps_uart->clkdiv = UART_DIV(115200) ;
		_gps_empty(true);
	}

	/* Configure GPS to be GPS-only (no GLONASS/BEIDOU) */
	gps_send("PCAS04,1");
#endif
}


// ---------------------------------------------------------------------------
// Test logic
// ---------------------------------------------------------------------------

static const struct {
	uint16_t val_hi;
	uint16_t val_lo;
} test_steps[5] = {
	{ 2048, 2048 }, /* 0 */
	{ 2048,    0 }, /* 1 */
	{ 2048, 4095 }, /* 2 */
	{    0, 2048 }, /* 3 */
	{ 4095, 2048 }, /* 4 */
};

static struct {
	int step;
	int freq_diff[5];
	int rep_count;
} test;

static void
test_init(void)
{
	/* State init */
	memset(&test, 0x00, sizeof(test));

	/* Apply DAC */
	pdm_set(PDM_CLK_HI, true, test_steps[test.step].val_hi, false);
	pdm_set(PDM_CLK_LO, true, test_steps[test.step].val_lo, false);
}

static bool
limit(int val, int min, int max)
{
	return (val >= min) && (val <= max);
}

static void
test_done(void)
{
	bool gok = true, ok;
	int d_lo_n = test.freq_diff[1] - test.freq_diff[0];
	int d_lo_p = test.freq_diff[2] - test.freq_diff[0];
	int d_hi_n = test.freq_diff[3] - test.freq_diff[0];
	int d_hi_p = test.freq_diff[4] - test.freq_diff[0];

	printf("\n");

	ok = limit(test.freq_diff[0], -100, 100); gok &= ok;
	printf("[+] Base             : %d - %s\n", test.freq_diff[0], ok ? "PASS" : "FAIL");

	ok = limit(-d_hi_n, 2304, 3840); gok &= ok;
	printf("[+] Hi Negative swing: %d - %s\n", d_hi_n, ok ? "PASS" : "FAIL");

	ok = limit( d_hi_p, 2304, 3840); gok &= ok;
	printf("[+] Hi Positive swing:  %d - %s\n", d_hi_p, ok ? "PASS" : "FAIL");

	ok = limit(-d_lo_n, 37, 62); gok &= ok;
	printf("[+] Lo Negative swing:   %d - %s\n", d_lo_n, ok ? "PASS" : "FAIL");

	ok = limit( d_lo_p, 37, 62); gok &= ok;
	printf("[+] Lo Positive swing:    %d - %s\n", d_lo_p, ok ? "PASS" : "FAIL");

	printf("\n");
	printf(gok ? "+++ PASS +++\n" : "!!! FAIL !!!");
	printf("\n");
}

static void
test_report(int diff_cur)
{
	/* Are we done ? */
	if (test.step == 5)
		return;

	/* Increment count */
	if (test.rep_count++ < 5)
		return;

	/* Debug */
	printf("[.] %d / %d -> %d Hz\n",
		test_steps[test.step].val_hi,
		test_steps[test.step].val_lo,
		diff_cur
	);

	/* Record result */
	test.freq_diff[test.step] = diff_cur;

	/* Reset for next step */
	test.rep_count = 0;
	test.step++;

	if (test.step == 5) {
		test_done();
		pdm_set(PDM_CLK_HI, true, 2048, false);
		pdm_set(PDM_CLK_LO, true, 2048, false);
	} else {
		pdm_set(PDM_CLK_HI, true, test_steps[test.step].val_hi, false);
		pdm_set(PDM_CLK_LO, true, test_steps[test.step].val_lo, false);
	}
}

// ---------------------------------------------------------------------------
// GPSDO measurement
// ---------------------------------------------------------------------------

#define GPSDO_MAX_OFFSET	10000
#define GPSDO_MAX_CHANGE	100

struct {
	uint32_t pps_last;	/* Last PPS time */
	int      diff_last;	/* Last frequency error measurement */
} gpsdo;

static void
pps_poll(void)
{
	uint32_t pps_now = time_pps_read();

	/* Any change ? */
	if (pps_now == gpsdo.pps_last)
		return;

	/* Compute frequency error */
	int diff_cur = ((pps_now - gpsdo.pps_last) & 0x7fffffff) - 30720000;

	/* Validate measurement */
	if ((abs(diff_cur) < GPSDO_MAX_OFFSET) &&
	    (abs(diff_cur - gpsdo.diff_last) < GPSDO_MAX_CHANGE)) {
		// printf("PPS freq diff: %d Hz\n", diff_cur);
		test_report(diff_cur);
	}

	/* Update */
	gpsdo.pps_last = pps_now;
	gpsdo.diff_last = diff_cur;
}


// ---------------------------------------------------------------------------
// Misc
// ---------------------------------------------------------------------------

extern const struct usb_stack_descriptors app_stack_desc;

static void
serial_no_init()
{
	uint8_t buf[8];
	char *id, *desc;
	int i;

	flash_manuf_id(buf);
	printf("Flash Manufacturer : %s\n", hexstr(buf, 3, true));

	flash_unique_id(buf);
	printf("Flash Unique ID    : %s\n", hexstr(buf, 8, true));

	/* Overwrite descriptor string */
		/* In theory in rodata ... but nothing is ro here */
	id = hexstr(buf, 8, false);
	desc = (char*)app_stack_desc.str[1];
	for (i=0; i<16; i++)
		desc[2 + (i << 1)] = id[i];
}

static void
boot_dfu(void)
{
	/* Force re-enumeration */
	usb_disconnect();

	/* Boot firmware */
	volatile uint32_t *boot = (void*)(MISC_BASE);
	*boot = (1 << 2) | (1 << 0);
}

void
usb_dfu_rt_cb_reboot(void)
{
        boot_dfu();
}


// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

void main()
{
	int cmd = 0;

	/* Init console IO */
	console_init();
	puts("Booting Test image..\n");

	/* LED */
	led_init();
	led_state(true);

	/* SPI */
	spi_init();
	serial_no_init();

	/* Enable LED now that we're done with SPI */
	e1_led_set(true, 0x00);

	/* Setup E1 Vref */
	int d = 10;
	pdm_set(PDM_E1_RX0,  true, 128 + d, false);
	pdm_set(PDM_E1_RX1,  true, 128 + d, false);

	/* Setup clock tuning */
	pdm_set(PDM_CLK_HI, true, 2048, false);
	pdm_set(PDM_CLK_LO, true, 2048, false);

	/* E1 init */
	for (int i=0; i<2; i++) {
		e1_init(i, E1_RX_CR_MODE_MFA, E1_TX_CR_MODE_TS0_CRC_E | E1_TX_CR_TICK_LOCAL);
		e1_start(i);
	}

	/* GPS init */
	gps_init();

	/* Enable USB directly */
	usb_init(&app_stack_desc);
	usb_dfu_rt_init();

	usb_connect();

	/* Start */
	bool gps_copy = false;

	test_init();

	/* Main loop */
	while (1)
	{
		/* Prompt ? */
		if (cmd >= 0)
			printf("Command> ");

		/* Poll for command */
		cmd = getchar_nowait();

		if (cmd >= 0) {
			if (cmd > 32 && cmd < 127) {
				putchar(cmd);
				putchar('\r');
				putchar('\n');
			}

			switch (cmd)
			{
			case 'b':
				boot_dfu();
				break;
			case 't':
				printf("%u %u\n",
					time_pps_read(),
					time_now_read()
				);
				break;

			/* GPS controls */
			case 'r':
				gpio_out(3, false);
				break;
			case 'R':
				gpio_out(3, true);
				break;
			case 'g':
				gps_copy ^= 1;
				break;

			/* DAC controls */
			case '0':
				pdm_set(PDM_CLK_HI, true,    0, false);
				break;
			case '1':
				pdm_set(PDM_CLK_HI, true, 2048, false);
				break;
			case '2':
				pdm_set(PDM_CLK_HI, true, 4095, false);
				break;
			case '3':
				pdm_set(PDM_CLK_LO, true,    0, false);
				break;
			case '4':
				pdm_set(PDM_CLK_LO, true, 2048, false);
				break;
			case '5':
				pdm_set(PDM_CLK_LO, true, 4095, false);
				break;

			default:
				break;
			}
		}

		/* USB poll */
		usb_poll();

		/* Copy GPS UART */
		uint32_t c;
		while (1) {
			c = gps_uart->data;
			if (c & 0x80000000)
				break;
			if (gps_copy)
				putchar(c);
		}

		/* Report clock */
		pps_poll();

		/* E1 poll */
		e1_poll(0);
		e1_poll(1);
	}
}
