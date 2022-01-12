/*
 * gps.c
 *
 * Copyright (C) 2019-2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "console.h"
#include "gps.h"
#include "misc.h"
#include "usb_gps.h"
#include "utils.h"

#include "config.h"


struct gps_uart {
	uint32_t data;
	uint32_t csr;
} __attribute__((packed,aligned(4)));

#define GPS_UART_CSR_DIV(baud)	((SYS_CLK_FREQ+(baud)/2)/(baud)-2)
#define GPS_UART_CSR_TX_EMPTY	(1 << 29)
#define GPS_UART_DATA_EMPTY	(1 << 31)

static volatile struct gps_uart * const gps_uart_regs = (void*)(GPS_UART_BASE);


static struct {
	/* NMEA rx state */
	struct {
		enum {
			GNS_WAIT   = 0,
			GNS_READ   = 1,
			GNS_CK_HI  = 2,
			GNS_CK_LO  = 3,
			GNS_END_CR = 4,
			GNS_END_LF = 5,
		} state;

		int     len;
		uint8_t cksum;
		char    buf[95];	/* Max length is actually 82 + 1 (\0) */
	} nmea;

	/* Current lock state */
	struct {
		bool valid;
	} fix;

	/* Reported antenna state */
	enum gps_antenna_state antenna;
} g_gps;


static void
_gps_empty(bool wait_eol)
{
	bool eol = false;

	uint32_t c;
	while (1) {
		c = gps_uart_regs->data;
		if (c & GPS_UART_DATA_EMPTY) {
			if (!wait_eol || eol)
				break;
		} else {
			eol = (c == '\n');
		}
	}
}

static void
_gps_send(const char *s)
{
	char cksum = 0;

	/* Start sentence */
	gps_uart_regs->data = '$';

	/* Send payload */
	while (*s)
		cksum ^= (gps_uart_regs->data = *s++);

	/* Send checksum */
	gps_uart_regs->data = '*';

	s = hexstr(&cksum, 1, false);
	gps_uart_regs->data = *s++;
	gps_uart_regs->data = *s++;

	gps_uart_regs->data = '\r';
	gps_uart_regs->data = '\n';
}

static const char *
_gps_query(void)
{
	uint32_t c;
	while (1) {
		/* Get next char */
		c = gps_uart_regs->data;
		if (c & GPS_UART_DATA_EMPTY)
			break;

		/* Always store it */
		if (g_gps.nmea.len == sizeof(g_gps.nmea.buf))
			g_gps.nmea.state = GNS_WAIT;
		else
			g_gps.nmea.buf[g_gps.nmea.len++] = c;

		/* State */
		if (c == '$') {
			/* '$' always triggers reset */
			g_gps.nmea.state  = GNS_READ;
			g_gps.nmea.cksum  = 0;
			g_gps.nmea.len    = 1;
			g_gps.nmea.buf[0] = '$';
		} else {
			switch (g_gps.nmea.state) {
			case GNS_READ:
				if (c == '*')
					g_gps.nmea.state = GNS_CK_HI;
				else
					g_gps.nmea.cksum ^= c;
				break;
			case GNS_CK_HI:
				g_gps.nmea.cksum ^= hexval(c) << 4;
				g_gps.nmea.state = GNS_CK_LO;
				break;
			case GNS_CK_LO:
				g_gps.nmea.cksum ^= hexval(c);
				g_gps.nmea.state = GNS_END_CR;
				break;
			case GNS_END_CR:
				g_gps.nmea.state = (c == '\r') ? GNS_END_LF : GNS_WAIT;
				break;
			case GNS_END_LF:
				g_gps.nmea.state = GNS_WAIT;
				g_gps.nmea.buf[g_gps.nmea.len] = 0x00;
				if (c == '\n')
					return g_gps.nmea.buf;
				break;
			default:
				g_gps.nmea.state = GNS_WAIT;
				g_gps.nmea.len   = 0;
				break;
			}
		}
	}
	return NULL;
}

void
_gps_parse_nmea(const char *nmea)
{
	/* Very basic parsing, we just look at GSA messages and consider
	 * that if we have a 3D fix with PDOP < 5, the timing data should
	 * be usable
	 */
	if (!strncmp(nmea, "$GPGSA", 6))
	{
		/* Check for Autonomous 3D fix (fixed field positions) */
		if ((nmea[7] != 'A') || (nmea[9] != '3')) {
			g_gps.fix.valid = false;
			return;
		}

		/* Find PDOP */
		const char *p = nmea;
		for (int i=0; (i < 15) && (*p != '*'); i += (*(p++)==',') );

		/* Is it low enough ? */
		g_gps.fix.valid = (p[1] < '5');
	}

	/* Parse TXT ANTENNA Status */
	if (!strncmp(nmea, "$GPTXT", 6) && nmea[13] == '0' && nmea[14] == '1')
	{
		if      (!strncmp(&nmea[16], "ANTENNA OK",    10))
			g_gps.antenna = ANT_OK;
		else if (!strncmp(&nmea[16], "ANTENNA OPEN",  12))
			g_gps.antenna = ANT_OPEN;
		else if (!strncmp(&nmea[16], "ANTENNA SHORT", 13))
			g_gps.antenna = ANT_SHORT;
		else
			g_gps.antenna = ANT_UNKNOWN;
	}
}


bool
gps_has_valid_fix(void)
{
	return g_gps.fix.valid;
}

enum gps_antenna_state
gps_antenna_status(void)
{
	return g_gps.antenna;
}

void
gps_poll(void)
{
	const char *nmea;

	/* Check if we have anything from the module */
	nmea = _gps_query();
	if (!nmea)
		return;

	/* If we do, process it locally to update our state */
	_gps_parse_nmea(nmea);

	/* And queue it for USB */
	usb_gps_puts(nmea);
}

void
gps_init(void)
{
	int i;

	/* State init */
	memset(&g_gps, 0x00, sizeof(g_gps));

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
		gps_uart_regs->csr = i ? GPS_UART_CSR_DIV(115200) : GPS_UART_CSR_DIV(9600);
		_gps_empty(false);

		/* Wait 100 ms */
		delay(100);

		/* Release reset line */
		gpio_out(3, true);

		/* Wait for first line of output as sign it's ready, timeout after 1s */
		while (!time_elapsed(start_time, SYS_CLK_FREQ))
			if ((init_ok = (_gps_query() != NULL)))
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

	/* If success was at 9600 baud, need to speed up */
	if (i == 0) {
		/* Configure GPS to use serial at 115200 baud */
		_gps_send("PCAS01,5");

		/* Add dummy byte which will be mangled during baudrate switch ... */
		gps_uart_regs->data = 0x00;
		while (!(gps_uart_regs->csr & GPS_UART_CSR_TX_EMPTY));

		/* Set uart to 115200 and empty uart buffer, line aligned */
		gps_uart_regs->csr = GPS_UART_CSR_DIV(115200) ;
		_gps_empty(true);
	}

	/* Configure GPS to be GPS-only (no GLONASS/BEIDOU) */
	_gps_send("PCAS04,1");
}
