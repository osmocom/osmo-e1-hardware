/*
 * utils.c
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include "console.h"
#include "led.h"
#include "mini-printf.h"
#include "misc.h"

char *
hexstr(void *d, int n, bool space)
{
	static const char * const hex = "0123456789abcdef";
	static char buf[96];
	uint8_t *p = d;
	char *s = buf;
	char c;

	while (n--) {
		c = *p++;
		*s++ = hex[c >> 4];
		*s++ = hex[c & 0xf];
		if (space)
			*s++ = ' ';
	}

	s[space?-1:0] = '\0';

	return buf;
}


void
_panic(const char *file, int lineno, const char *fmt, ...)
{
	char buf[256];
	va_list va;
	int l;

	/* Fast hard red blinking led */
	led_state(true);
	led_color(255, 0, 8);
	led_breathe(false, 0, 0);
	led_blink(true, 75, 75);

	/* Prepare buffer */
	l = mini_snprintf(buf, 255, "PANIC @ %s:%d = ", file, lineno);

	va_start(va, fmt);
	l += mini_vsnprintf(buf+l, 255-l, fmt, va);
	va_end(va);

	buf[l] = '\n';
	buf[l+1] = '\0';

	/* Print once */
	puts(buf);

	/* Loop waiting for commands */
	while (1) {
		int cmd = getchar_nowait();

		switch (cmd) {
		case 'b':
			/* Reboot */
			reboot(2);
			break;
		case ' ':
			/* Print error again */
			puts(buf);
			break;
		}
	}
}
