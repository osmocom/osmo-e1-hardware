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
#include "misc.h"
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
e1f_init(struct e1_fifo *fifo, unsigned int base, unsigned int len)
{
	memset(fifo, 0x00, sizeof(struct e1_fifo));
	fifo->base = base;
	fifo->mask = len - 1;
}

static void
e1f_reset(struct e1_fifo *fifo)
{
	fifo->wptr[0] = fifo->wptr[1] = 0;
	fifo->rptr[0] = fifo->rptr[1] = 0;
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
e1f_multiframe_empty_tail(struct e1_fifo *fifo)
{
	fifo->rptr[0] = fifo->rptr[1] = (fifo->wptr[0] & ~15);
}

static void
e1f_multiframe_empty_head(struct e1_fifo *fifo)
{
	fifo->wptr[0] = fifo->wptr[1] = ((fifo->rptr[1] + 15) & ~15);
}


// Main logic
// ----------

enum e1_pipe_state {
	IDLE	 = 0,	/* not running */
	STARTING = 1,	/* after e1_start(), waiting for priming */
	RUN	 = 2,	/* normal operation */
	RECOVER	 = 3,	/* after underflow, overflow or alignment  error */
	SHUTDOWN = 4,	/* after e1_stop(), waiting for shutdown */
};

struct e1_state {
	struct {
		struct {
			uint32_t cfg;
			uint32_t val;
		} cr;
		struct e1_fifo fifo;
		int in_flight;
		enum e1_pipe_state state;
	} rx;

	struct {
		struct {
			uint32_t cfg;
			uint32_t val;
		} cr;
		struct e1_fifo fifo;
		int in_flight;
		enum e1_pipe_state state;
	} tx;

	struct {
		uint16_t rx_pulse;
		uint16_t rx_sample;
		uint16_t rx_one;

		uint16_t _val;
	} linemon;

	struct e1_error_count errors;

	struct {
		enum e1_platform_led_state green;
		enum e1_platform_led_state yellow;
	} led;
};

static struct e1_state g_e1[NUM_E1_PORTS];


static volatile struct e1_core *
_get_regs(int port)
{
	if ((port < 0) || (port >= NUM_E1_PORTS))
		panic("_get_regs invalid port %d", port);
	return &e1_regs_base[port];
}

static struct e1_state *
_get_state(int port)
{
	if ((port < 0) || (port >= NUM_E1_PORTS))
		panic("_get_state invalid port %d", port);
	return &g_e1[port];
}


#define RXCR_PERMITTED (			\
		E1_RX_CR_MODE_MASK )

#define TXCR_PERMITTED (			\
		E1_TX_CR_MODE_MASK |		\
		E1_TX_CR_TICK_MASK |		\
		E1_TX_CR_ALARM |		\
		E1_TX_CR_LOOPBACK |		\
		E1_TX_CR_LOOPBACK_CROSS )

static void
_e1_update_cr_val(int port)
{
	struct e1_state *e1 = _get_state(port);

	/* RX */
	if (e1->rx.state == IDLE) {
		/* "Off" state: Force MFA mode to detect remote side */
		e1->rx.cr.val = (e1->rx.cr.cfg & ~E1_RX_CR_MODE_MASK) | E1_RX_CR_ENABLE | E1_RX_CR_MODE_MFA;
	} else {
		/* "On state: Enabled + User config */
		e1->rx.cr.val = e1->rx.cr.cfg | E1_RX_CR_ENABLE;
	}

	/* TX */
	if (e1->tx.state == IDLE) {
		/* "Off" state: We TX only AIS */
		e1->tx.cr.val = (e1->tx.cr.cfg & ~(E1_TX_CR_MODE_MASK | E1_TX_CR_ALARM)) | E1_TX_CR_ENABLE | E1_TX_CR_MODE_TRSP;
	} else {
		/* "On state: Enabled + User config */
		e1->tx.cr.val = e1->tx.cr.cfg | E1_TX_CR_ENABLE;
	}
}

static void
_e1_update_leds(int port)
{
	struct e1_state *e1 = _get_state(port);

	enum e1_platform_led_state green;
	enum e1_platform_led_state yellow;

	/*
	 * Green	Yellow		Condition
	 * OFF		/		LOS
	 * BLINK	OFF		LOF
	 * BLINK	ON		LOF + AIS
	 * ON		OFF		ALIGNED
	 * ON		ON		ALIGNED + RAI
	 */

	/* Compute expected state */
	if (e1->errors.flags & E1_ERR_F_LOS) {
		green  = E1P_LED_ST_OFF;
		yellow = E1P_LED_ST_OFF;
	} else if (e1->errors.flags & E1_ERR_F_ALIGN_ERR) {
		green  = E1P_LED_ST_BLINK;
		yellow = (e1->errors.flags & E1_ERR_F_AIS) ? E1P_LED_ST_ON : E1P_LED_ST_OFF;
	} else {
		green  = E1P_LED_ST_ON;
		yellow = (e1->errors.flags & E1_ERR_F_RAI) ? E1P_LED_ST_ON : E1P_LED_ST_OFF;
	}

	/* Update actual leds */
	if (e1->led.green != green)
		e1_platform_led_set(port, E1P_LED_GREEN, green);

	if (e1->led.yellow != yellow)
		e1_platform_led_set(port, E1P_LED_YELLOW, yellow);

	e1->led.green  = green;
	e1->led.yellow = yellow;

	/* Update the shared RGB led */
	if (port == (NUM_E1_PORTS - 1))
	{
		static bool c_ok = true,  c_flow = false;
		bool        n_ok = false, n_flow = false;

		for (int port=0; port<NUM_E1_PORTS; port++)
		{
			struct e1_state *e1 = _get_state(port);
			n_ok   |= !(e1->errors.flags & E1_ERR_F_ALIGN_ERR);
			n_flow |= (e1->rx.state != IDLE);
			n_flow |= (e1->tx.state != IDLE);
		}

		if (n_ok != c_ok) {
			if (n_ok)
				led_color(0, 48, 0); // Green
			else
				led_color(48, 0, 0); // Red
		}

		if (n_flow != c_flow) {
			led_blink(n_flow, 200, 1000);
			led_breathe(n_flow, 100, 200);
		}

		c_ok   = n_ok;
		c_flow = n_flow;
	}
}

void
e1_init(int port, uint16_t rx_cr, uint16_t tx_cr)
{
	volatile struct e1_core *e1_regs = _get_regs(port);
	struct e1_state *e1 = _get_state(port);

	/* Global state init */
	memset(e1, 0x00, sizeof(struct e1_state));

	/* Initialize FIFOs */
	e1f_init(&e1->rx.fifo, (512 * port) +   0, 256);
	e1f_init(&e1->tx.fifo, (512 * port) + 256, 256);

	/* Flow state */
	e1->rx.state = IDLE;
	e1->tx.state = IDLE;

	/* Set config registers */
	e1->rx.cr.cfg = rx_cr & RXCR_PERMITTED;
	e1->tx.cr.cfg = tx_cr & TXCR_PERMITTED;

	_e1_update_cr_val(port);

	e1_regs->rx.csr = e1->rx.cr.val;
	e1_regs->tx.csr = e1->tx.cr.val;
}

void
e1_rx_config(int port, uint16_t cr)
{
	volatile struct e1_core *e1_regs = _get_regs(port);
	struct e1_state *e1 = _get_state(port);
	e1->rx.cr.cfg = cr & RXCR_PERMITTED;
	_e1_update_cr_val(port);
	e1_regs->rx.csr = e1->rx.cr.val;
}

void
e1_tx_config(int port, uint16_t cr)
{
	volatile struct e1_core *e1_regs = _get_regs(port);
	struct e1_state *e1 = _get_state(port);
	e1->tx.cr.cfg = cr & TXCR_PERMITTED;
	_e1_update_cr_val(port);
	e1_regs->tx.csr = e1->tx.cr.val;
}

void
e1_start(int port)
{
	volatile struct e1_core *e1_regs = _get_regs(port);
	struct e1_state *e1 = _get_state(port);

	/* RX */
	switch (e1->rx.state) {
	case IDLE:
		/* We're idle, clear fifo and normal start */
		e1f_reset(&e1->rx.fifo);
		e1->rx.state = STARTING;
		break;

	case SHUTDOWN:
		/* Shutdown is pending, go to recover which is basically
		 * a shutdown with auto-restart */
		e1->rx.state = RECOVER;
		break;

	default:
		/* Huh ... hope for the best */
		printf("[!] E1 RX start while not stopped ...\n");
	}

	/* TX */
	switch (e1->tx.state) {
	case IDLE:
		/* We're idle, clear fifo and normal start */
		e1f_reset(&e1->tx.fifo);
		e1->tx.state = STARTING;
		break;

	case SHUTDOWN:
		/* Shutdown is pending, go to recover which is basically
		 * a shutdown with auto-restart */
		e1->tx.state = RECOVER;

		/* We also prune any pending data in FIFO that's not
		 * already queued to hw */
		e1f_multiframe_empty_head(&e1->rx.fifo);
		break;

	default:
		/* Huh ... hope for the best */
		printf("[!] E1 TX start while not stopped ...\n");
	}

	/* Update CRs */
	_e1_update_cr_val(port);

	e1_regs->rx.csr = e1->rx.cr.val | E1_RX_CR_OVFL_CLR;
	e1_regs->tx.csr = e1->tx.cr.val | E1_TX_CR_UNFL_CLR;
}

void
e1_stop(int port)
{
	struct e1_state *e1 = _get_state(port);

	/* Flow state */
	e1->rx.state = SHUTDOWN;
	e1->tx.state = SHUTDOWN;

	/* Nothing else to do, e1_poll will stop submitting data and
	 * transition to IDLE when everything in-flight is done */
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
		if (rai_received)
			e1->errors.flags |= E1_ERR_F_RAI;
		else
			e1->errors.flags &= ~E1_ERR_F_RAI;
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
e1_rx_level(int port)
{
	struct e1_state *e1 = _get_state(port);
	return e1f_valid_frames(&e1->rx.fifo);
}

unsigned int
e1_tx_level(int port)
{
	struct e1_state *e1 = _get_state(port);
	return e1f_valid_frames(&e1->tx.fifo);
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

	/* Update error flags */
	if (e1_regs->rx.csr & E1_RX_SR_ALIGNED) {
		e1->errors.flags &= ~(
			E1_ERR_F_LOS |
			E1_ERR_F_AIS |
			E1_ERR_F_RAI |
			E1_ERR_F_ALIGN_ERR
		);
	} else {
		e1->errors.flags |=  E1_ERR_F_ALIGN_ERR;
		e1->errors.flags &= ~E1_ERR_F_RAI;
	}

	/* If we have any local alarm, make sure to notify remote side */
	if (e1->errors.flags & (E1_ERR_F_LOS | E1_ERR_F_AIS | E1_ERR_F_ALIGN_ERR))
		e1_regs->tx.csr = e1->tx.cr.val | E1_TX_CR_ALARM;
	else
		e1_regs->tx.csr = e1->tx.cr.val;

	/* Update leds */
	_e1_update_leds(port);

	/* Active ? */
	if ((e1->rx.state == IDLE) && (e1->tx.state == IDLE))
		return;

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
	if (e1->tx.state == STARTING) {
		if (e1f_unseen_frames(&e1->tx.fifo) < (16 * 5))
			return;
	}

	/* Handle RX */
		/* Bypass if OFF */
	if (e1->rx.state == IDLE)
		goto done_rx;

		/* Shutdown */
	if (e1->rx.state == SHUTDOWN) {
		if (e1->rx.in_flight == 0) {
			e1->rx.state = IDLE;
			_e1_update_cr_val(port);
			e1_regs->rx.csr = e1->rx.cr.val;
		}
		goto done_rx;
	}

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
		e1f_multiframe_empty_tail(&e1->rx.fifo);
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
		e1_regs->rx.csr = e1->rx.cr.val | E1_RX_CR_OVFL_CLR;
		e1->rx.state = RUN;
	}
done_rx:

	/* Handle TX */
		/* Bypass if OFF */
	if (e1->tx.state == IDLE)
		return;

		/* Shutdown */
	if (e1->tx.state == SHUTDOWN) {
		if (e1->tx.in_flight == 0) {
			e1->tx.state = IDLE;
			_e1_update_cr_val(port);
			e1_regs->tx.csr = e1->tx.cr.val;
		}
		return;
	}

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
		e1_regs->tx.csr = e1->tx.cr.val | E1_TX_CR_UNFL_CLR;
		e1->tx.state = RUN;
	}
}

void
e1_linemon_update(void)
{
	static int cycle = -1;

	/* Initial boot */
	if (cycle == -1) {
		e1_tick_sel(TICK_RX_PULSE);
		cycle = 0;
		return;
	}

	/* Current cycle ? */
	switch (cycle) {
		/* Read initial values */
	case 0:
	case 2:
	case 4:
		for (int port=0; port<NUM_E1_PORTS; port++)
			g_e1[port].linemon._val = e1_tick_read(port);
		break;

		/* Actual reading */
	case 1:
		for (int port=0; port<NUM_E1_PORTS; port++)
			g_e1[port].linemon.rx_pulse = e1_tick_read(port) - g_e1[port].linemon._val;
		e1_tick_sel(TICK_RX_SAMPLE);
		break;

	case 3:
		for (int port=0; port<NUM_E1_PORTS; port++)
			g_e1[port].linemon.rx_sample = e1_tick_read(port) - g_e1[port].linemon._val;
		e1_tick_sel(TICK_RX_ONE);
		break;

	case 5:
		for (int port=0; port<NUM_E1_PORTS; port++)
			g_e1[port].linemon.rx_one = e1_tick_read(port) - g_e1[port].linemon._val;
		e1_tick_sel(TICK_RX_PULSE);
		break;
	}

	/* Next cycle */
	if (++cycle == 6)
	{
		/* We did one full cycle of data, update our local flags */
		for (int port=0; port<NUM_E1_PORTS; port++)
		{
			struct e1_state *e1 = _get_state(port);
			if (e1->linemon.rx_pulse < 16) {
				/* No pulse ?  -> LOS */
				e1->errors.flags |=  E1_ERR_F_LOS;
				e1->errors.flags &= ~E1_ERR_F_AIS;
			} else {
				/* We have "some" pulses, so somone is talking */
				e1->errors.flags &= ~E1_ERR_F_LOS;

				/* If it's mostly ones, consider it AIS */
				if (e1->linemon.rx_one > 2040)
					e1->errors.flags |=  E1_ERR_F_AIS;
				else
					e1->errors.flags &= ~E1_ERR_F_AIS;
			}
		}

		/* Start over */
		cycle = 0;
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
	printf("Tck: P %d / S %d / O %d\n",
		e1->linemon.rx_pulse, e1->linemon.rx_sample, e1->linemon.rx_one);

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
