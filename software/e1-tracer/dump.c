/* (C) 2019 by Harald Welte <laforge@gnumonks.org>
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

#include <osmocom/core/utils.h>
#include <osmocom/core/logging.h>
#include <osmocom/core/application.h>
#include <osmocom/core/isdnhdlc.h>
#include <osmocom/core/gsmtap.h>
#include <osmocom/core/gsmtap_util.h>
//#include <osmocom/abis/lapd_pcap.h>

#include "osmo_e1f.h"

#define E1_CHUNK_HDR_MAGIC	0xe115600d /* E1 is good */
struct e1_chunk_hdr {
	uint32_t magic;
	struct {
		uint64_t sec;
		uint64_t usec;
	} time;
	uint16_t len;		/* length of following payload */
	uint8_t ep;		/* USB endpoint */
} __attribute__((packed));


/* local state per E1 line */
struct line_state {
	unsigned int idx;
	struct osmo_isdnhdlc_vars hdlc;
	struct osmo_e1f_instance e1f;
	struct {
		FILE *outfile;
		uint64_t last_sec;
		uint64_t frame_err_cnt;
		uint64_t crc_err_cnt;
		bool ts0_crc4_error;
		bool ts0_remote_alarm;
	} errplot;
};

static struct line_state g_line[2]; /* one per direction */
static int g_pcap_fd = -1;
static struct msgb *g_pcap_msg;
struct gsmtap_inst *g_gsmtap;

/* called for each HDLC payload frame */
static void handle_payload(uint8_t idx, const uint8_t *data, int len)
{
#if 0
	int dir;

	switch (idx) {
	case 0:
		dir = OSMO_LAPD_PCAP_INPUT;
		break;
	case 1:
		dir = OSMO_LAPD_PCAP_OUTPUT;
		break;
	default:
		fprintf(stderr, "Unexpected USB EP idx %d\n", idx);
		return;
	}

	if (g_pcap_fd >= 0) {
		uint8_t *cur = msgb_put(g_pcap_msg, len);
		memcpy(cur, data, len);
		osmo_pcap_lapd_write(g_pcap_fd, dir, g_pcap_msg);
		msgb_reset(g_pcap_msg);
	} else
#endif
		printf("%u: OUT: %s\n", idx, osmo_hexdump(data, len));

	if (g_gsmtap) {
		struct msgb *msg;
		msg = gsmtap_makemsg_ex(GSMTAP_TYPE_E1T1, idx ? GSMTAP_ARFCN_F_UPLINK : 0, 255,
					GSMTAP_E1T1_FR, 0, 0, 0, 0, data, len);
		OSMO_ASSERT(msg);
		gsmtap_sendmsg(g_gsmtap, msg);
	}
}



static void handle_frame_errplot(struct line_state *ls, const struct e1_chunk_hdr *hdr, const uint8_t *data)
{
	if (!ls->errplot.outfile)
		return;

	if (!ls->errplot.last_sec)
		ls->errplot.last_sec = hdr->time.sec;

	if (ls->errplot.last_sec != hdr->time.sec) {
		/* dump the per-second total; start from 0 again */
		fprintf(ls->errplot.outfile, "%"PRIu64 " %"PRIu64 " %"PRIu64" %u %u\n",
			hdr->time.sec, ls->errplot.frame_err_cnt, ls->errplot.crc_err_cnt,
			ls->errplot.ts0_crc4_error, ls->errplot.ts0_remote_alarm);
		ls->errplot.frame_err_cnt = 0;
		ls->errplot.crc_err_cnt = 0;
		ls->errplot.ts0_remote_alarm = false;
		ls->errplot.ts0_crc4_error = false;
		ls->errplot.last_sec = hdr->time.sec;
		fflush(ls->errplot.outfile);
	}
}

/* called for each USB transfer read from the file */
static void handle_frame(const struct e1_chunk_hdr *hdr, const uint8_t *data)
{
	uint8_t nots0[1024];
	unsigned int offs = 0;
	struct line_state *ls;

	/* filter on the endpoint (direction) specified by the user */
	switch (hdr->ep) {
	case 0x81:
		ls = &g_line[0];
		break;
	case 0x82:
		ls = &g_line[1];
		break;
	default:
		fprintf(stderr, "Unexpected USB EP 0x%02x\n", hdr->ep);
		return;
	}

	if (hdr->len <= 4)
		return;

	//printf("%u: %"PRIu64".%"PRIu64" EP=0x%02x\n", ls->idx, hdr->time.sec, hdr->time.usec, hdr->ep);

	OSMO_ASSERT(((hdr->len-4)/32)*31 < ARRAY_SIZE(nots0));
	/* gather the TS1..TS31 data, skipping TS0 */
	for (int i = 4; i < hdr->len-4; i += 32) {
		//printf("%u:\t%s\n", ls->idx, osmo_hexdump(data+i, 32));
		memcpy(nots0+offs, data+i+1, 32-1);
		offs += 31;
		osmo_e1f_rx_frame(&ls->e1f, data+i);
	}

	//printf("%u: IN(%u): %s\n", ls->idx, offs, osmo_hexdump(nots0, offs));
	uint8_t out[2048];
	int rc;
	int rl;

	int oi = 0;

	while (oi < offs) {
		rc = osmo_isdnhdlc_decode(&ls->hdlc, nots0+oi, offs-oi, &rl, out, sizeof(out));
		//printf("%u: osmo_isdnhdlc_decode(hdlc, nots0+%d, inlen=%d, &rl=%d, out, %zu)=%d\n", ls->idx, oi, offs-oi, rl, sizeof(out), rc);
		if (rc < 0) {
			fprintf(stdout, "%u: ERR in HDLC decode: %d\n", ls->idx, rc);
			if (rc == -1)
				ls->errplot.frame_err_cnt++;
			else if (rc == -2)
				ls->errplot.crc_err_cnt++;
		} else if (rc > 0)
			handle_payload(ls->idx, out, rc);
		oi += rl;
	}
	handle_frame_errplot(ls, hdr, data);
}

static int process_file(int fd)
{
	struct e1_chunk_hdr hdr;
	unsigned long offset = 0;
	uint8_t buf[65535];
	int rc;

	while (1) {
		memset(buf, 0, sizeof(buf));
		/* first read header */
		rc = read(fd, &hdr, sizeof(hdr));
		if (rc < 0)
			return rc;
		if (rc != sizeof(hdr)) {
			fprintf(stderr, "%d is less than header size (%zd)\n", rc, sizeof(hdr));
			return -1;
		}
		offset += rc;
		if (hdr.magic != E1_CHUNK_HDR_MAGIC) {
			fprintf(stderr, "offset %lu: Wrong magic 0x%08x\n", offset, hdr.magic);
			return -1;
		}

		/* then read payload */
		rc = read(fd, buf, hdr.len);
		if (rc < 0)
			return rc;
		offset += rc;
		if (rc != hdr.len) {
			fprintf(stderr, "%d is less than payload size (%d)\n", rc, hdr.len);
			return -1;
		}
		handle_frame(&hdr, buf);
	}
}

static int open_file(const char *fname)
{
	return open(fname, O_RDONLY);
}

/* E1 framer notifies us of something */
static void notify_cb(struct osmo_e1f_instance *e1i, enum osmo_e1f_notify_event evt, bool present, void *data)
{
	struct line_state *ls = e1i->priv;

	printf("%u: NOTIFY: %s %s\n", ls->idx, osmo_e1f_notify_event_name(evt),
		present ? "PRESENT" : "ABSENT");

	if (present) {
		switch (evt) {
		case E1_NTFY_EVT_CRC_ERROR:
			ls->errplot.ts0_crc4_error = present;
			break;
		case E1_NTFY_EVT_REMOTE_ALARM:
			ls->errplot.ts0_remote_alarm = present;
			break;
		}
	}
}

static const struct log_info_cat log_categories[] = {
};

static const struct log_info log_info = {
	.cat = log_categories,
	.num_cat = ARRAY_SIZE(log_categories),
};

int main(int argc, char **argv)
{
	char *fname;
	int rc;
	int i;

	osmo_init_logging2(NULL, &log_info);
	osmo_e1f_init();

	if (argc < 2) {
		fprintf(stderr, "You must specify the file name of the ICE40-E1 capture\n");
		exit(1);
	}
	fname = argv[1];

	rc = open_file(fname);
	if (rc < 0) {
		fprintf(stderr, "Error opening %s: %s\n", fname, strerror(errno));
		exit(1);
	}

	g_gsmtap = gsmtap_source_init("localhost", GSMTAP_UDP_PORT, 0);
	gsmtap_source_add_sink(g_gsmtap);

	if (argc >= 3) {
#if 0
		g_pcap_fd = osmo_pcap_lapd_open(argv[2], 0640);
		if (g_pcap_fd < 0) {
			fprintf(stderr, "Unable to open PCAP output: %s\n", strerror(errno));
			exit(1);
		}
		g_pcap_msg = msgb_alloc(4096, "pcap");
#endif
	}

	for (i = 0; i < ARRAY_SIZE(g_line); i++) {
		struct line_state *ls = &g_line[i];
		char namebuf[32];
		ls->idx = i;
		osmo_isdnhdlc_rcv_init(&ls->hdlc, OSMO_HDLC_F_BITREVERSE);
		osmo_e1f_instance_init(&ls->e1f, "dump", &notify_cb, true, ls);

		snprintf(namebuf, sizeof(namebuf), "errplot-%d.dat", i);
		ls->errplot.outfile = fopen(namebuf, "w");
	}

	process_file(rc);
}
