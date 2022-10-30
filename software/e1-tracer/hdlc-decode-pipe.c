/* (C) 2020 by Harald Welte <laforge@gnumonks.org>
 * All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

#include <osmocom/core/isdnhdlc.h>

static struct osmo_isdnhdlc_vars g_hdlc;

/* read bitstream from STDIN, pass through HDLC decoder, write decoded frames to stdout */

int main(int argc, char **argv)
{
	osmo_isdnhdlc_rcv_init(&g_hdlc, OSMO_HDLC_F_BITREVERSE);

	while (1) {
		uint8_t inbuf[320];
		uint8_t outbuf[2048];
		int rc, inlen, outlen;

		rc = read(0, inbuf, sizeof(inbuf));
		if (rc < 0)
			exit(1);
		else if (rc == 0)
			exit(0);

		inlen = rc;
		rc = osmo_isdnhdlc_decode(&g_hdlc, inbuf, inlen, &outlen, outbuf, sizeof(outbuf));

		if (outlen > 0)
			write(1, outbuf, outlen);
	}
}
