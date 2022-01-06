/*
 * e1.c
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "config.h"
#include "console.h"
#include "e1.h"
#include "e1_hw.h"

#include "dma.h"
#include "led.h" // FIXME
#include "utils.h"


// HW access
// ---------

static volatile struct e1_core * const e1_regs_base = (void *)(E1_CORE_BASE);
static volatile uint8_t * const e1_data = (void *)(E1_DATA_BASE);


// Helpers
// -------

static unsigned int
e1_data_ofs(int mf, int frame, int ts)
{
	return (mf << 9) | (frame << 5) | ts;
}

static volatile uint8_t *
e1_data_ptr(int mf, int frame, int ts)
{
	return &e1_data[e1_data_ofs(mf, frame, ts)];
}


// FIFOs
// -----
/* Note: FIFO works at 'frame' level (i.e. 32 bytes) */

struct e1_fifo {
	/* Buffer zone associated with the FIFO */
	unsigned int base;
	unsigned int mask;

	/* Pointers / Levels */
	unsigned int wptr[2];	/* 0=committed 1=allocated */
	unsigned int rptr[2];	/* 0=discared  1=peeked    */
};

	/* Utils */
static void
e1f_reset(struct e1_fifo *fifo, unsigned int base, unsigned int len)
{
	memset(fifo, 0x00, sizeof(struct e1_fifo));
	fifo->base = base;
	fifo->mask = len - 1;
}

static unsigned int
e1f_allocd_frames(struct e1_fifo *fifo)
{
	/* Number of frames that are allocated (i.e. where we can't write to) */
	return (fifo->wptr[1] - fifo->rptr[0]) & fifo->mask;
}

static unsigned int
e1f_valid_frames(struct e1_fifo *fifo)
{
	/* Number of valid frames */
	return (fifo->wptr[0] - fifo->rptr[0]) & fifo->mask;
}

static unsigned int
e1f_unseen_frames(struct e1_fifo *fifo)
{
	/* Number of valid frames that haven't been peeked yet */
	return (fifo->wptr[0] - fifo->rptr[1]) & fifo->mask;
}

static unsigned int
e1f_free_frames(struct e1_fifo *fifo)
{
	/* Number of frames that aren't allocated */
	return (fifo->rptr[0] - fifo->wptr[1] - 1) & fifo->mask;
}

static unsigned int
e1f_ofs_to_dma(unsigned int ofs)
{
	/* DMA address are 32-bits word address. Offsets are 32 byte address */
	return (ofs << 3);
}

static unsigned int
e1f_ofs_to_mf(unsigned int ofs)
{
	/* E1 Buffer Descriptors are always multiframe aligned */
	return (ofs >> 4);
}

	/* Debug */
static void
e1f_debug(struct e1_fifo *fifo, const char *name)
{
	unsigned int la, lv, lu, lf;

	la = e1f_allocd_frames(fifo);
	lv = e1f_valid_frames(fifo);
	lu = e1f_unseen_frames(fifo);
	lf = e1f_free_frames(fifo);

	printf("%s: R: %u / %u | W: %u / %u | A:%u  V:%u  U:%u  F:%u\n",
		name,
		fifo->rptr[0], fifo->rptr[1], fifo->wptr[0], fifo->wptr[1],
		la, lv, lu, lf
	);
}

	/* Frame level read/write */
static unsigned int
e1f_frame_write(struct e1_fifo *fifo, unsigned int *ofs, unsigned int max_frames)
{
	unsigned int lf, le;

	lf = e1f_free_frames(fifo);
	le = fifo->mask - fifo->wptr[0] + 1;

	if (max_frames > le)
		max_frames = le;
	if (max_frames > lf)
		max_frames = lf;

	*ofs = fifo->base + fifo->wptr[0];
	fifo->wptr[1] = fifo->wptr[0] = (fifo->wptr[0] + max_frames) & fifo->mask;

	return max_frames;
}

static unsigned int
e1f_frame_read(struct e1_fifo *fifo, unsigned int *ofs, unsigned int max_frames)
{
	unsigned int lu, le;

	lu = e1f_unseen_frames(fifo);
	le = fifo->mask - fifo->rptr[1] + 1;

	if (max_frames > le)
		max_frames = le;
	if (max_frames > lu)
		max_frames = lu;

	*ofs = fifo->base + fifo->rptr[1];
	fifo->rptr[0] = fifo->rptr[1] = (fifo->rptr[1] + max_frames) & fifo->mask;

	return max_frames;
}


	/* MultiFrame level split read/write */
static bool
e1f_multiframe_write_prepare(struct e1_fifo *fifo, unsigned int *ofs)
{
	unsigned int lf;

	lf = e1f_free_frames(fifo);
	if (lf < 16)
		return false;

	*ofs = fifo->base + fifo->wptr[1];
	fifo->wptr[1] = (fifo->wptr[1] + 16) & fifo->mask;

	return true;
}

static void
e1f_multiframe_write_commit(struct e1_fifo *fifo)
{
	fifo->wptr[0] = (fifo->wptr[0] + 16) & fifo->mask;
}

static bool
e1f_multiframe_read_peek(struct e1_fifo *fifo, unsigned int *ofs)
{
	unsigned int lu;

	lu = e1f_unseen_frames(fifo);
	if (lu < 16)
		return false;

	*ofs = fifo->base + fifo->rptr[1];
	fifo->rptr[1] = (fifo->rptr[1] + 16) & fifo->mask;

	return true;
}

static void
e1f_multiframe_read_discard(struct e1_fifo *fifo)
{
	fifo->rptr[0] = (fifo->rptr[0] + 16) & fifo->mask;
}

static void
e1f_multiframe_empty(struct e1_fifo *fifo)
{
	fifo->rptr[0] = fifo->rptr[1] = (fifo->wptr[0] & ~15);
}


// Main logic
// ----------

enum e1_pipe_state {
	IDLE	= 0,	/* not yet initialized */
	BOOT	= 1,	/* after e1_init(), regiters are programmed */
	RUN	= 2,	/* normal operation */
	RECOVER	= 3,	/* after underflow, overflow or alignment  error */
};

struct e1_state {
	struct {
		uint32_t cr;
		struct e1_fifo fifo;
		int in_flight;
		enum e1_pipe_state state;
	} rx;

	struct {
		uint32_t cr;
		struct e1_fifo fifo;
		int in_flight;
		enum e1_pipe_state state;
	} tx;

	struct e1_error_count errors;
};

static struct e1_state g_e1[2];


static volatile struct e1_core *
_get_regs(int port)
{
	if ((port < 0) || (port > 1))
		panic("_get_regs invalid port %d", port);
	return &e1_regs_base[port];
}

static struct e1_state *
_get_state(int port)
{
	if ((port < 0) || (port > 1))
		panic("_get_state invalid port %d", port);
	return &g_e1[port];
}


void
e1_init(int port, uint16_t rx_cr, uint16_t tx_cr)
{
	volatile struct e1_core *e1_regs = _get_regs(port);
	struct e1_state *e1 = _get_state(port);

	/* Global state init */
	memset(e1, 0x00, sizeof(struct e1_state));

	/* Reset FIFOs */
	e1f_reset(&e1->rx.fifo, (512 * port) +   0, 256);
	e1f_reset(&e1->tx.fifo, (512 * port) + 256, 256);

	/* Enable Rx */
	e1->rx.cr = E1_RX_CR_ENABLE | rx_cr;
	e1_regs->rx.csr = E1_RX_CR_OVFL_CLR | e1->rx.cr;

	/* Enable Tx */
	e1->tx.cr = E1_TX_CR_ENABLE | tx_cr;
	e1_regs->tx.csr = E1_TX_CR_UNFL_CLR | e1->tx.cr;

	/* State */
	e1->rx.state = BOOT;
	e1->tx.state = BOOT;
}

#define TXCR_PERMITTED (			\
		E1_TX_CR_MODE_TS0_CRC_E	|	\
		E1_TX_CR_TICK_REMOTE |		\
		E1_TX_CR_ALARM	|		\
		E1_TX_CR_LOOPBACK |		\
		E1_TX_CR_LOOPBACK_CROSS	)

void
e1_tx_config(int port, uint16_t cr)
{
	volatile struct e1_core *e1_regs = _get_regs(port);
	struct e1_state *e1 = _get_state(port);
	e1->tx.cr = (e1->tx.cr & ~TXCR_PERMITTED) | (cr & TXCR_PERMITTED);
	e1_regs->tx.csr = e1->tx.cr;
}

#define RXCR_PERMITTED (			\
		E1_RX_CR_MODE_MFA )

void
e1_rx_config(int port, uint16_t cr)
{
	volatile struct e1_core *e1_regs = _get_regs(port);
	struct e1_state *e1 = _get_state(port);
	e1->rx.cr = (e1->rx.cr & ~RXCR_PERMITTED) | (cr & RXCR_PERMITTED);
	e1_regs->rx.csr = e1->rx.cr;
}

unsigned int
e1_rx_need_data(int port, unsigned int usb_addr, unsigned int max_frames, unsigned int *pos)
{
	struct e1_state *e1 = _get_state(port);
	bool rai_received = false;
	bool rai_possible = false;
	unsigned int ofs;
	int tot_frames = 0;
	int n_frames, i;

	while (max_frames) {
		/* Get some data from the FIFO */
		n_frames = e1f_frame_read(&e1->rx.fifo, &ofs, max_frames);
		if (!n_frames)
			break;

		/* Give pos */
		if (pos) {
			*pos = ofs & e1->rx.fifo.mask;
			pos = NULL;
		}

		/* Copy from FIFO to USB */
		dma_exec(e1f_ofs_to_dma(ofs), usb_addr, n_frames * (32 / 4), false, NULL, NULL);

		/* Prepare Next */
		usb_addr += n_frames * (32 / 4);
		max_frames -= n_frames;
		tot_frames += n_frames;

		/* While DMA is running: Determine if remote end indicates any alarms */
		for (i = 0; i < n_frames; i++) {
			unsigned int frame_nr = ofs + i;
			/* A bit is present in every odd frame TS0 */
			if (frame_nr & 1) {
				uint8_t ts0 = *e1_data_ptr(0, ofs + i, 0);
				rai_possible = true;
				if (ts0 & 0x20) {
					rai_received = true;
					break;
				}
			}
		}

		/* Wait for DMA completion */
		while (dma_poll());
	}

	if (rai_possible) {
		if (rai_received) {
			e1->errors.flags |= E1_ERR_F_RAI;
			e1_platform_led_set(port, E1P_LED_YELLOW, E1P_LED_ST_ON);
		} else {
			e1->errors.flags &= ~E1_ERR_F_RAI;
			e1_platform_led_set(port, E1P_LED_YELLOW, E1P_LED_ST_OFF);
		}
	}

	return tot_frames;
}

unsigned int
e1_tx_feed_data(int port, unsigned int usb_addr, unsigned int frames)
{
	struct e1_state *e1 = _get_state(port);
	unsigned int ofs;
	int n_frames;

	while (frames) {
		/* Get some space in FIFO */
		n_frames = e1f_frame_write(&e1->tx.fifo, &ofs, frames);
		if (!n_frames) {
			printf("[!] TX FIFO Overflow (port=%d, req=%d, done=%d)\n", port, frames, n_frames);
			e1f_debug(&e1->tx.fifo, "TX");
			break;
		}

		/* Copy from USB to FIFO */
		dma_exec(e1f_ofs_to_dma(ofs), usb_addr, n_frames * (32 / 4), true, NULL, NULL);

		/* Prepare next */
		usb_addr += n_frames * (32 / 4);
		frames -= n_frames;

		/* Wait for DMA completion */
		while (dma_poll());
	}

	return frames;
}

unsigned int
e1_tx_level(int port)
{
	struct e1_state *e1 = _get_state(port);
	return e1f_valid_frames(&e1->tx.fifo);
}

unsigned int
e1_rx_level(int port)
{
	struct e1_state *e1 = _get_state(port);
	return e1f_valid_frames(&e1->rx.fifo);
}

const struct e1_error_count *
e1_get_error_count(int port)
{
	struct e1_state *e1 = _get_state(port);
	return &e1->errors;
}

void
e1_poll(int port)
{
	volatile struct e1_core *e1_regs = _get_regs(port);
	struct e1_state *e1 = _get_state(port);
	uint32_t bd;
	unsigned int ofs;

	/* Active ? */
	if ((e1->rx.state == IDLE) && (e1->tx.state == IDLE))
		return;

	/* HACK: LED link status */
	if (e1_regs->rx.csr & E1_RX_SR_ALIGNED) {
		e1_platform_led_set(port, E1P_LED_GREEN, E1P_LED_ST_ON);
		led_color(0, 48, 0);
		e1->errors.flags &= ~(E1_ERR_F_LOS|E1_ERR_F_ALIGN_ERR);
	} else {
		e1_platform_led_set(port, E1P_LED_GREEN, E1P_LED_ST_BLINK);
		e1_platform_led_set(port, E1P_LED_YELLOW, E1P_LED_ST_OFF);
		led_color(48, 0, 0);
		e1->errors.flags |= E1_ERR_F_ALIGN_ERR;
		/* TODO: completely off if rx tick counter not incrementing */
	}

	/* Recover any done TX BD */
	while ( (bd = e1_regs->tx.bd) & E1_BD_VALID ) {
		e1f_multiframe_read_discard(&e1->tx.fifo);
		e1->tx.in_flight--;
	}

	/* Recover any done RX BD */
	while ( (bd = e1_regs->rx.bd) & E1_BD_VALID ) {
		/* FIXME: CRC status ? */
		e1f_multiframe_write_commit(&e1->rx.fifo);
		if ((bd & (E1_BD_CRC0 | E1_BD_CRC1)) != (E1_BD_CRC0 | E1_BD_CRC1)) {
			printf("[!] E1 crc err (port=%d, bd=%03x)\n", port, bd);
			e1->errors.crc++;
		}
		e1->rx.in_flight--;
	}

	/* Boot procedure */
	if (e1->tx.state == BOOT) {
		if (e1f_unseen_frames(&e1->tx.fifo) < (16 * 5))
			return;
		/* HACK: LED flow status */
		led_blink(true, 200, 1000);
		led_breathe(true, 100, 200);
	}

	/* Handle RX */
		/* Misalign ? */
	if (e1->rx.state == RUN) {
		if (!(e1_regs->rx.csr & E1_RX_SR_ALIGNED)) {
			printf("[!] E1 rx misalign (port=%d)\n", port);
			e1->rx.state = RECOVER;
			e1->errors.align++;
		}
	}

		/* Overflow ? */
	if (e1->rx.state == RUN) {
		if (e1_regs->rx.csr & E1_RX_SR_OVFL) {
			printf("[!] E1 overflow (port=%d, inf=%d)\n", port, e1->rx.in_flight);
			e1->rx.state = RECOVER;
			e1->errors.ovfl++;
		}
	}

		/* Recover ready ? */
	if (e1->rx.state == RECOVER) {
		if (e1->rx.in_flight != 0)
			goto done_rx;
		e1f_multiframe_empty(&e1->rx.fifo);
	}

		/* Fill new RX BD */
	while (e1->rx.in_flight < 4) {
		if (!e1f_multiframe_write_prepare(&e1->rx.fifo, &ofs))
			break;
		e1_regs->rx.bd = e1f_ofs_to_mf(ofs);
		e1->rx.in_flight++;
	}

		/* Clear overflow if needed */
	if (e1->rx.state != RUN) {
		e1_regs->rx.csr = e1->rx.cr | E1_RX_CR_OVFL_CLR;
		e1->rx.state = RUN;
	}
done_rx:

	/* Handle TX */
		/* Underflow ? */
	if (e1->tx.state == RUN) {
		if (e1_regs->tx.csr & E1_TX_SR_UNFL) {
			printf("[!] E1 underflow (port=%d, inf=%d)\n", port, e1->tx.in_flight);
			e1->tx.state = RECOVER;
			e1->errors.unfl++;
		}
	}

		/* Recover ready ? */
	if (e1->tx.state == RECOVER) {
		if (e1f_unseen_frames(&e1->tx.fifo) < (16 * 5))
			return;
	}

		/* Fill new TX BD */
	while (e1->tx.in_flight < 4) {
		if (!e1f_multiframe_read_peek(&e1->tx.fifo, &ofs))
			break;
		e1_regs->tx.bd = e1f_ofs_to_mf(ofs);
		e1->tx.in_flight++;
	}

		/* Clear underflow if needed */
	if (e1->tx.state != RUN) {
		e1_regs->tx.csr = e1->tx.cr | E1_TX_CR_UNFL_CLR;
		e1->tx.state = RUN;
	}
}

void
e1_debug_print(int port, bool data)
{
	volatile struct e1_core *e1_regs = _get_regs(port);
	struct e1_state *e1 = _get_state(port);
	volatile uint8_t *p;

	printf("E1 port %d\n", port);
	printf("CSR: Rx %04x / Tx %04x\n", e1_regs->rx.csr, e1_regs->tx.csr);
	printf("InF: Rx %d / Tx %d\n", e1->rx.in_flight, e1->tx.in_flight);
	printf("Sta: Rx %d / Tx %d\n", e1->rx.state, e1->tx.state);

	e1f_debug(&e1->rx.fifo, "Rx FIFO");
	e1f_debug(&e1->tx.fifo, "Tx FIFO");

	if (data) {
		puts("\nE1 Data\n");
		for (int f=0; f<16; f++) {
			p = e1_data_ptr(0, f, 0);
			for (int ts=0; ts<32; ts++)
				printf(" %02x", p[ts]);
			printf("\n");
		}
	}
}
