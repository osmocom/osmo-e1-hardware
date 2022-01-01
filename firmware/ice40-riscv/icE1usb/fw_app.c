/*
 * fw_app.c
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <no2usb/usb.h>
#include <no2usb/usb_dfu_rt.h>

#include "config.h"
#include "console.h"
#include "e1.h"
#include "led.h"
#include "misc.h"
#include "mini-printf.h"
#include "spi.h"
#include "usb_e1.h"
#include "utils.h"


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
	reboot(1);
}

void
usb_dfu_rt_cb_reboot(void)
{
        boot_dfu();
}

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

uint8_t
hexval(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return 10 + (c - 'a');
	else if (c >= 'A' && c <= 'F')
		return 10 + (c - 'A');
	else
		return 0;
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

#if 1
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
// GPSDO measurement
// ---------------------------------------------------------------------------

#define GPSDO_MAX_OFFSET	10000
#define GPSDO_MAX_CHANGE	100

struct {
	uint32_t pps_last;	/* Last PPS time */
	int      diff_last;	/* Last frequency error measurement */
} gpsdo;

#define HI_STEEPNESS	1678	/* 1.6782 counts per step */

int cur_hi_val = 2048;
int cur_lo_val = 2085;
int total_diff = 0;


static void
pps_poll(void)
{
	uint32_t pps_now = time_pps_read();
	static uint32_t cnt = 0;
	int coarse = 0;
	static int up = 0, down = 0, same = 0;
	static int vote_cnt = 0;
	static int cur_interval = 10;
	static int same_cnt = 0;

	/* Any change ? */
	if (pps_now == gpsdo.pps_last)
		return;

	/* Compute frequency error */
	int diff_cur = ((pps_now - gpsdo.pps_last) & 0x7fffffff) - 30720000;

	/* Validate measurement */
	if ((abs(diff_cur) < GPSDO_MAX_OFFSET) &&
	    (abs(diff_cur - gpsdo.diff_last) < GPSDO_MAX_CHANGE)) {
		cnt++;

		/* Set the hi-value for a coarse correction */
		if (cnt == 5) {
			coarse = (diff_cur * 1000) / HI_STEEPNESS;
			cur_hi_val -= coarse;
			pdm_set(PDM_CLK_HI, true, cur_hi_val, false);
		}

		/* Perform fine correction */
		if (cnt > 20) {
			if (diff_cur == 0)
				same++;
			else if (diff_cur >= 1)
				down += diff_cur;
			else
				up -= diff_cur;

			if (up > cur_interval || down > cur_interval || same > cur_interval) {
				if (up > down && up > same) {
					cur_lo_val++;
				} else if (down > up && down > same) {
					cur_lo_val--;
				} else {
					if (abs(up - down) > cur_interval/8) {
						if (up > down)
							cur_lo_val++;
						else
							cur_lo_val--;
					}
				}

				pdm_set(PDM_CLK_LO, true, cur_lo_val, false);

				/* we are settled in the current state,
				 * switch to the next higher integration intverval */
				if (same > cur_interval/2) {
					if (cur_interval == 10)
						cur_interval = 100;
					else if (cur_interval == 100)
						cur_interval = 600;
				}

				up = same = down = 0;
			}

			total_diff += diff_cur;
		}

		printf("PPS freq diff: %d Hz, cur_hi_val: %d, cur_lo_val: %d hi_corr, "
		       "coarse: %d, total_diff: %d | ", diff_cur, cur_hi_val,  cur_lo_val, coarse, total_diff);
		printf("down %d, same %d, up %d\n", down, same, up);
	}

	/* Update */
	gpsdo.pps_last = pps_now;
	gpsdo.diff_last = diff_cur;
}


void main()
{
	bool e1_active = false;
	int cmd = 0;

	/* Init console IO */
	console_init();
	puts("Booting App image..\n");

	/* LED */
	led_init();

	/* SPI */
	spi_init();
	serial_no_init();

	/* Enable LED now that we're done with SPI */
	e1_led_set(true, 0x00);

	/* Setup E1 Vref */
	int d = 25;
#if defined(BOARD_ICE1USB_PROTO_ICEBREAKER) || defined(BOARD_ICE1USB_PROTO_BITSY)
	pdm_set(PDM_E1_CT, true, 128, false);
	pdm_set(PDM_E1_P,  true, 128 - d, false);
	pdm_set(PDM_E1_N,  true, 128 + d, false);
#else
	pdm_set(PDM_E1_RX0,  true, 128 + d, false);
	pdm_set(PDM_E1_RX1,  true, 128 + d, false);
#endif

	/* Setup clock tuning */
	pdm_set(PDM_CLK_HI, true, 2048, false);
	pdm_set(PDM_CLK_LO, false,   0, false);

	/* Enable USB directly */
	usb_init(&app_stack_desc);
	usb_dfu_rt_init();
	usb_e1_init();

	/* Start */
	e1_init(0, 0);
	e1_active = true;

	/* GPS init */
	gps_init();

	led_state(true);
	usb_connect();

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
			case 'p':
				usb_debug_print();
				break;
			case 'b':
				boot_dfu();
				break;
			case 'o':
				e1_debug_print(false);
				break;
			case 'O':
				e1_debug_print(true);
				break;
			case 'c':
				usb_connect();
				break;
			case 'd':
				usb_disconnect();
				break;
			default:
				break;
			}
		}

		/* USB poll */
		usb_poll();

		/* E1 poll */
		if (e1_active) {
			e1_poll();
			usb_e1_run();
		}

		/* Report clock */
		pps_poll();
	}
}
