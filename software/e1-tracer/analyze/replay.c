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
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include <assert.h>

#include <osmocom/core/utils.h>

#define E1_TS_BITRATE	64000
#define E1_31TS_BITRATE	(31*E1_TS_BITRATE)
#define E1_31TS_BYTERATE (E1_31TS_BITRATE/8)

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


static struct timespec ts_start;
static uint64_t bytes_total;

static uint64_t get_msecs_total(void)
{
	struct timespec ts_now;
	clock_gettime(CLOCK_MONOTONIC, &ts_now);
	return (ts_now.tv_sec*1000 + ts_now.tv_nsec/1000000) - (ts_start.tv_sec*1000 + ts_start.tv_nsec/1000000);
}

/* called for each USB transfer read from the file */
static void handle_frame(const struct e1_chunk_hdr *hdr, const uint8_t *data)
{
	uint8_t nots0[1024];
	unsigned int offs = 0;

	/* filter on the endpoint (direction) specified by the user */
	switch (hdr->ep) {
	case 0x81:
		break;
	case 0x82:
		return;
	default:
		fprintf(stderr, "Unexpected USB EP 0x%02x\n", hdr->ep);
		return;
	}

	if (hdr->len <= 4)
		return;

	//printf("%u: %"PRIu64".%"PRIu64" EP=0x%02x\n", ls->idx, hdr->time.sec, hdr->time.usec, hdr->ep);

	assert(((hdr->len-4)/32)*31 < ARRAY_SIZE(nots0));
	/* gather the TS1..TS31 data, skipping TS0 */
	for (int i = 4; i < hdr->len-4; i += 32) {
		//printf("%u:\t%s\n", ls->idx, osmo_hexdump(data+i, 32));
		memcpy(nots0+offs, data+i+1, 32-1);
		offs += 31;
		write(1, data+i, 31);
		bytes_total += 31;
	}

	/* check number of bytes written / number of seconds expired -> sleep or not? */
	uint64_t msecs_total = get_msecs_total();
	if (msecs_total == 0)
		return;
	uint64_t bytes_per_sec = bytes_total * 1000 / msecs_total;
	//fprintf(stderr, "%"PRIu64" msecs, %"PRIu64" bytes, %"PRIu64" bytes/s\n", msecs_total, bytes_total, bytes_per_sec);
	if (bytes_per_sec > E1_31TS_BYTERATE) {
		uint64_t msecs_expected = bytes_total * 1000 / E1_31TS_BYTERATE;
		//fprintf(stderr, "expected: %"PRIu64" msecs, actual: %"PRIu64" msecs, sleepint\n", msecs_expected, msecs_total);
		usleep((msecs_expected - msecs_total)*1000);
	}
}

static int process_file(void *map, off_t len)
{
	unsigned long offset = 0;

	clock_gettime(CLOCK_MONOTONIC, &ts_start);

	while (offset < len) {
		struct e1_chunk_hdr *hdr = map + offset;

		offset += sizeof(*hdr);
		if (hdr->magic != E1_CHUNK_HDR_MAGIC) {
			fprintf(stderr, "offset %lu: Wrong magic 0x%08x\n", offset, hdr->magic);
			return -1;
		}

		/* then read payload */
		handle_frame(hdr, (const uint8_t *)map + offset);
		offset += hdr->len;
	}

	return 0;
}

static void *open_mmap_file(const char *fname, off_t *size)
{
	int fd, rc;
	struct stat st;
	void *map;

	fd = open(fname, O_RDONLY);
	if (fd < 0)
		return NULL;
	rc = fstat(fd, &st);
	if (rc < 0) {
		close(fd);
		return NULL;
	}

	map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);

	if (map == MAP_FAILED)
		return NULL;

	if (size)
		*size = st.st_size;
	return map;
}

int main(int argc, char **argv)
{
	char *fname;
	void *map;
	off_t map_len;

	if (argc < 2) {
		fprintf(stderr, "You must specify the file name of the ICE40-E1 capture\n");
		exit(1);
	}
	fname = argv[1];

	map = open_mmap_file(fname, &map_len);
	if (!map) {
		fprintf(stderr, "Error opening %s: %s\n", fname, strerror(errno));
		exit(1);
	}

	process_file(map, map_len);
}
