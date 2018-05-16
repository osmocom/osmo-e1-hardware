/* Osmocom Software Defined E1
 *
 * (C) 2018 by Harald Welte <laforge@gnumonks.org>
 *
 * Implements ITU-T Rec. G.704 Section 2.3
 */

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <osmocom/core/msgb.h>
#include <osmocom/core/linuxlist.h>
#include <osmocom/core/logging.h>
#include <osmocom/core/fsm.h>

#include "crc4itu.h"
#include "osmo_e1f.h"

#define S(x)	(1 << (x))

/* Frame Alignment Signal (BIT1 may be overwritten with CRC-4) */
#define G704_E1_FAS	0x1B

static inline bool is_correct_fas(uint8_t bt) {
	if ((bt & 0x7F) == G704_E1_FAS)
		return true;
	else
		return false;
}

/* are we in SMF II (true) or I (false) */
static inline bool is_smf_II(const struct osmo_e1f_tx_state *tx) {
	if (tx->frame_nr >= 8)
		return true;
	return false;
}

static struct osmo_fsm e1_align_fsm;
static void align_fsm_reset(struct osmo_e1f_instance *e1i);

static void notify_user(struct osmo_e1f_instance *e1i, enum osmo_e1f_notify_event evt,
			bool present, void *priv)
{
	if (!e1i->notify_cb)
		return;
	e1i->notify_cb(e1i, evt, present, priv);
}

/*! Initialize a (caller-allocated) Osmocom E1 Instance
 *  \param[inout] e1i E1 Instance to be initialized
 *  \returns 0 on success, negative on error */
int osmo_e1f_instance_init(struct osmo_e1f_instance *e1i, const char *name, e1_notify_cb cb,
			  bool crc4_enabled, void *priv)
{
	int i;

	e1i->crc4_enabled = crc4_enabled;
	e1i->notify_cb = cb;
	e1i->tx.sa4_sa8 = 0x00;

	e1i->priv = priv;

	for (i = 1; i < ARRAY_SIZE(e1i->ts); i++) {
		struct osmo_e1f_instance_ts *e1t = &e1i->ts[i];
		e1t->ts_nr = i;
		e1t->inst = e1i;
		INIT_LLIST_HEAD(&e1t->tx.queue);

		e1t->rx.granularity = 256;
	}

	e1i->rx.fi = osmo_fsm_inst_alloc(&e1_align_fsm, NULL, e1i, LOGL_DEBUG, name);
	if (!e1i->rx.fi)
		return -1;

	osmo_e1f_instance_reset(e1i);

	return 0;
}

/*! stop E1 timeslot; release any pending rx/tx buffers
 *  \param[in] e1t Timeslot which we are to stop, disable and release buffers */
void osmo_e1f_ts_reset(struct osmo_e1f_instance_ts *e1t)
{
	e1t->tx.underruns = 0;
	msgb_queue_free(&e1t->tx.queue);

	e1t->rx.enabled = false;
	msgb_free(e1t->rx.msg);
	e1t->rx.msg = NULL;

	osmo_isdnhdlc_rcv_init(&e1t->rx.hdlc, OSMO_HDLC_F_BITREVERSE);
	//osmo_isdnhdlc_rcv_init(&e1t->rx.hdlc, 0);
	osmo_isdnhdlc_out_init(&e1t->tx.hdlc, 0);
}

/*! stop E1 instance; stops all timeslots and releases any pending rx/tx buffers
 *  \param[in] e1t E1 instance which we are to stop */
void osmo_e1f_instance_reset(struct osmo_e1f_instance *e1i)
{
	int i;

	align_fsm_reset(e1i);

	e1i->tx.remote_alarm = false;
	e1i->tx.crc4_error = false;
	e1i->tx.frame_nr = 0;
	e1i->tx.crc4_last_smf = 0;
	e1i->tx.crc4 = crc4itu_init();

	e1i->rx.frame_nr = 0;
	memset(&e1i->rx.ts0_history, 0, sizeof(e1i->rx.ts0_history));
	e1i->rx.ts0_hist_len = 0;
	e1i->rx.remote_alarm = false;
	e1i->rx.remote_crc4_error = false;
	e1i->rx.num_ts0_in_mframe_search = 0;

	for (i = 1; i < ARRAY_SIZE(e1i->ts); i++) {
		struct osmo_e1f_instance_ts *e1t = &e1i->ts[i];
		osmo_e1f_ts_reset(e1t);
	}
}

/*! obtain pointer to TS given by instance + timeslot number
 *  \param[in] e1i E1 intance on which we work
 *  \param[in] ts_nr E1 timeslot number (1..31)
 *  \returns pointer to timeslot; NULL on error */
struct osmo_e1f_instance_ts *osmo_e1f_instance_ts(struct osmo_e1f_instance *e1i, uint8_t ts_nr)
{
	if (ts_nr == 0 || ts_nr >= ARRAY_SIZE(e1i->ts))
		return NULL;

	return &e1i->ts[ts_nr];
}

/*! configure an E1 timeslot
 *  \param[in] e1t Timeslot which we are to configure
 *  \param[in] granularity granularity (buffer size) to use on Rx
 *  \param[in] enable enable (true) or disalble (false) receiving on this TS
 *  \param[in] mode the mode for this timeslot (raw or hdlc)
 *  \return 0 on success; negative on error */
int osmo_e1f_ts_config(struct osmo_e1f_instance_ts *e1t, e1_data_cb cb, unsigned int granularity,
		      bool enable, enum osmo_e1f_ts_mode mode)
{
	e1t->rx.data_cb = cb;
	e1t->rx.enabled = enable;
	e1t->rx.granularity = granularity;
	e1t->mode = mode;

	return 0;
}

const struct value_string osmo_e1f_notifv_evt_names[] = {
	{ E1_NTFY_EVT_ALIGN_FRAME, "Aligned to Frame" },
	{ E1_NTFY_EVT_ALIGN_CRC_MFRAME, "Aligned to CRC4-Multiframe" },
	{ E1_NTFY_EVT_CRC_ERROR, "CRC Error detected (local)" },
	{ E1_NTFY_EVT_REMOTE_CRC_ERROR, "CRC Error reported (remote)" },
	{ E1_NTFY_EVT_REMOTE_ALARM, "Remote Alarm condition repoorted" },
	{ 0, NULL }
};

/***********************************************************************
 * Transmit Side
 ***********************************************************************/

/*! Enqueue a message buffer of to-be-transmitted data for a timeslot
 *  \param[in] e1i E1 instance for which to enqueue
 *  \param[in] ts_nr Timeslot number on which data is to be transmitted
 *  \param[in] msg Message buffer storing the to-be-transmitted data
 *  \returns 0 on success; negative in case of error.
 *
 *  Ownership of \a msg is transferred from caller into this function, but only
 *  in case of successful execution (return 0)!
 */
void osmo_e1f_ts_enqueue(struct osmo_e1f_instance_ts *e1t, struct msgb *msg)
{
	msgb_enqueue(&e1t->tx.queue, msg);
}

/* obtain a CRC4 bit for the current frame number */
static uint8_t e1_pull_crc4_bit(struct osmo_e1f_instance *e1i)
{
	/* If CRC-4 is disabled, all CRC bits shall be '1' */
	if (e1i->crc4_enabled == 0) {
		return 0x01;
	} else {
		/* CRC is transmitted MSB first */
		switch (e1i->tx.frame_nr % 8) {
		case 0:
			return (e1i->tx.crc4_last_smf >> 3) & 1;
		case 2:
			return (e1i->tx.crc4_last_smf >> 2) & 1;
		case 4:
			return (e1i->tx.crc4_last_smf >> 1) & 1;
		case 6:
			return (e1i->tx.crc4_last_smf >> 0) & 1;
		default:
			OSMO_ASSERT(0);
		}
	}
}

/* pull a single to-be-transmitted byte for TS0 */
static uint8_t e1_pull_ts0(struct osmo_e1f_instance *e1i)
{
	uint8_t ret;

	/* according to Table 5B/G.704 - CRC-4 multiframe structure */
	if ((e1i->tx.frame_nr % 2) == 0) {
		/* FAS */
		ret = G704_E1_FAS | (e1_pull_crc4_bit(e1i) << 7);
	} else {
		switch (e1i->tx.frame_nr) {
		case 1:
		case 3:
		case 7:
			ret = 0x40;
			break;
		case 5:
		case 9:
		case 11:
			ret = 0xC0;
			break;
		case 13:
		case 15:
			ret = 0x40;
			if (e1i->tx.crc4_error)
				ret |= 0x80;
			break;
		}
		ret |= e1i->tx.sa4_sa8;
		if (e1i->tx.remote_alarm)
			ret |= 0x20;
	}

	/* re-set CRC4 at start of sub-multiframe */
	if (e1i->tx.frame_nr == 0 || e1i->tx.frame_nr == 8) {
		e1i->tx.crc4_last_smf = e1i->tx.crc4;
		e1i->tx.crc4 = 0;
	}

	/* increment frame number modulo 16 */
	e1i->tx.frame_nr = (e1i->tx.frame_nr + 1) % 16;

	return ret;
}

/* pull a single to-be-transmitted byte for TS1..31 */
static uint8_t e1_pull_tsN(struct osmo_e1f_instance_ts *e1t)
{
	struct msgb *msg = llist_first_entry_or_null(&e1t->tx.queue, struct msgb, list);
	uint8_t *cur;

retry:
	/* if there's no message to transmit */
	if (!msg) {
		e1t->tx.underruns++;
		return 0xFF;
	}
	if (msgb_length(msg) <= 0) {
		llist_del(&msg->list);
		msgb_free(msg);
		msg = llist_first_entry_or_null(&e1t->tx.queue, struct msgb, list);
		goto retry;
	}
	cur = msgb_pull(msg, 1);
	return *cur;
}

/* update the current in-progress CRC4 value with data from \a out_frame */
static void e1_tx_update_crc4(struct osmo_e1f_instance *e1i, const uint8_t *out_frame)
{
	uint8_t ts0;

	ts0 = out_frame[0];
	/* mask off the C bits */
	if (is_correct_fas(ts0))
		ts0 &= 0x7F;
	e1i->tx.crc4 = crc4itu_update(e1i->tx.crc4, &ts0, 1);
	/* add the remaining bytes/bits */
	e1i->tx.crc4 = crc4itu_update(e1i->tx.crc4, out_frame+1, ARRAY_SIZE(e1i->ts)-1);
}

/*! Pull one to-be-transmitted E1 frame (256bits) from the E1 instance
 *  \param e1i E1 instance for which the frame shall be generated
 *  \param[out] out_frame callee-allocated buffer to which function stores 32 bytes
 *  \returns 0 on success, negative on error */
int osmo_e1f_pull_tx_frame(struct osmo_e1f_instance *e1i, uint8_t *out_frame)
{
	int i;

	/* generate TS0 */
	out_frame[0] = e1_pull_ts0(e1i);

	/* generate TS1..31 */
	for (i = 1; i < ARRAY_SIZE(e1i->ts); i++) {
		struct osmo_e1f_instance_ts *e1t = &e1i->ts[i];
		/* get next to-be-transmitted byte from the TS */
		out_frame[i] = e1_pull_tsN(e1t);
	}
	/* update our CRC4 computation */
	e1_tx_update_crc4(e1i, out_frame);

	return 0;
}

/***********************************************************************
 * Receiver Side
 ***********************************************************************/

/* According to Figure 2 / ITU-T G.706 */
enum e1_align_state {
	/* Frame Alignment Search */
	E1_AS_SEARCH_FRAME,
	/* CRC multiframe alignment search */
	E1_AS_SEARCH_CRC_MFRAME,
	/* monitoring for incorrect frame alignment and error performance using CRC */
	E1_AS_ALIGNED_CRC_MFRAME,
	/* no CRC: just frame alignment loss check */
	E1_AS_ALIGNED_BASIC,
};

enum e1_align_event {
	/* received a TS0 octet */
	E1_AE_RX_TS0,
	E1_AE_RESET
};

static const struct value_string e1_align_evt_names[] = {
	{ E1_AE_RX_TS0, "E1_AE_RX_TS0" },
	{ E1_AE_RESET, "E1_AE_RESET" },
	{ 0, NULL }
};

/* get a TS0 byte from the history. delta 0 == current, delte 1 == previous, ... */
static uint8_t get_ts0_hist(struct osmo_e1f_instance *e1i, uint8_t delta)
{
	return e1i->rx.ts0_history[((e1i->rx.frame_nr + 16)-delta) % 16];
}

/* ITU-T G.706 Section 4.1.1 */
static bool frame_alignment_lost(struct osmo_e1f_instance *e1i)
{
	if (e1i->rx.frame_nr % 2)
		return false;

	/* Frame alignment will be assumed to have been lost when three consecutive incorrect
	 * frame alignment signals have been received. */
	if (!is_correct_fas(get_ts0_hist(e1i, 0)) &&
	    !is_correct_fas(get_ts0_hist(e1i, 2)) &&
	    !is_correct_fas(get_ts0_hist(e1i, 4)))
		return true;
	else
		return false;
}

/* ITU-T G.706 Section 4.1.2 */
static bool frame_alignment_recovered(struct osmo_e1f_instance *e1i)
{
	/* two consecutive FAS with one non-FAS interspersed */
	if (is_correct_fas(get_ts0_hist(e1i, 0)) &&
	    !is_correct_fas(get_ts0_hist(e1i, 1)) &&
	    is_correct_fas(get_ts0_hist(e1i, 2)))
		return true;
	else
		return false;
}

/* ITU-T G.706 Section 4.2 */
static bool crc_mframe_alignment_achieved(struct osmo_e1f_instance *e1i)
{
	/* if current TS0 byte is FAS, we cannot detect alignment */
	if (is_correct_fas(get_ts0_hist(e1i, 0)))
		return false;
	if ((get_ts0_hist(e1i, 0) >> 7) == 1 &&
	    (get_ts0_hist(e1i, 2) >> 7) == 1 &&
	    (get_ts0_hist(e1i, 4) >> 7) == 0 &&
	    (get_ts0_hist(e1i, 6) >> 7) == 1 &&
	    (get_ts0_hist(e1i, 8) >> 7) == 0 &&
	    (get_ts0_hist(e1i, 10) >> 7) == 0)
		return true;
	else
		return false;
}

/* Get the CRC4 that was received from our Rx TS0 history */
static uint8_t crc4_from_ts0_hist(struct osmo_e1f_instance *e1i, bool smf2)
{
	uint8_t crc = 0;
	uint8_t offset = 0;

	if (smf2)
		offset = 8;

	crc |= (e1i->rx.ts0_history[0+offset] >> 7) << 3;
	crc |= (e1i->rx.ts0_history[2+offset] >> 7) << 2;
	crc |= (e1i->rx.ts0_history[4+offset] >> 7) << 1;
	crc |= (e1i->rx.ts0_history[6+offset] >> 7) << 0;

	return crc;
}

/* update the current in-progress CRC4 value with data from \a rx_frame */
static void e1_rx_update_crc4(struct osmo_e1f_instance *e1i, const uint8_t *rx_frame)
{
	uint8_t ts0;

	ts0 = rx_frame[0];
	/* mask off the C bits */
	if (is_correct_fas(ts0))
		ts0 &= 0x7F;
	e1i->rx.crc4 = crc4itu_update(e1i->rx.crc4, &ts0, 1);
	/* add the remaining bytes/bits */
	e1i->rx.crc4 = crc4itu_update(e1i->rx.crc4, rx_frame+1, ARRAY_SIZE(e1i->ts)-1);
}

/* FSM State handler */
static void e1_align_search_frame(struct osmo_fsm_inst *fi, uint32_t event, void *data)
{
	struct osmo_e1f_instance *e1i = (struct osmo_e1f_instance *) fi->priv;

	if (frame_alignment_recovered(e1i)) {
		/* if we detected the 2nd FAS, we must be in FN 2 (or at least FN%2=0 */
		e1i->rx.frame_nr = 2;
		notify_user(e1i, E1_NTFY_EVT_ALIGN_FRAME, true, NULL);
		osmo_fsm_inst_state_chg(fi, E1_AS_SEARCH_CRC_MFRAME, 0, 0);
	}
}

/* FSM State handler */
static void e1_align_search_crc_mframe(struct osmo_fsm_inst *fi, uint32_t event, void *data)
{
	struct osmo_e1f_instance *e1i = (struct osmo_e1f_instance *) fi->priv;

	if (crc_mframe_alignment_achieved(e1i)) {
		/* if we detected the 6-bit CRC multiframe signal, we must be in FN 11 */
		e1i->rx.frame_nr = 11;
		/* FIXME: "at least two valid CRC multiframe alignment signals can be located within
		 * 8 ms, the time separating two CRC multiframe alignment signals being 2 ms or a
		 * multiple of 2 ms" */
		notify_user(e1i, E1_NTFY_EVT_ALIGN_CRC_MFRAME, true, NULL);
		osmo_fsm_inst_state_chg(fi, E1_AS_ALIGNED_CRC_MFRAME, 0, 0);
	} else {
		/* if no mframe alignment is established within 8ms (64 frames), fall back */
		if (e1i->rx.num_ts0_in_mframe_search >= 64) {
			e1i->rx.num_ts0_in_mframe_search = 0;
			osmo_fsm_inst_state_chg(fi, E1_AS_SEARCH_FRAME, 0, 0);
		}
		e1i->rx.num_ts0_in_mframe_search++;
	}
}

static void e1_aligned_common(struct osmo_e1f_instance *e1i)
{
	uint8_t inb = get_ts0_hist(e1i, 0);

	/* All non-FAS frames contain "A" bit in TS0 */
	if (!is_correct_fas(inb & 0x7F)) {
		bool old_alarm = e1i->rx.remote_alarm;
		/* frame not containing the frame alignment signal */
		if (inb & 0x20)
			e1i->rx.remote_alarm = true;
		else
			e1i->rx.remote_alarm = false;
		if (old_alarm != e1i->rx.remote_alarm)
			notify_user(e1i, E1_NTFY_EVT_REMOTE_ALARM, e1i->rx.remote_alarm, NULL);
	}
}

/* FSM State handler */
static void e1_aligned_crc_mframe(struct osmo_fsm_inst *fi, uint32_t event, void *data)
{
	struct osmo_e1f_instance *e1i = (struct osmo_e1f_instance *) fi->priv;

	if (frame_alignment_lost(e1i)) {
		osmo_fsm_inst_state_chg(fi, E1_AS_SEARCH_FRAME, 0, 0);
		return;
	}

	if (e1i->crc4_enabled) {
		uint8_t crc_rx;
		bool crc4_error;

		/* check if we just received a complete CRC4 */
		switch (e1i->rx.frame_nr) {
		case 7:
		case 15:
			crc_rx = crc4_from_ts0_hist(e1i, e1i->rx.frame_nr == 15 ? true : false);
			if (crc_rx != e1i->rx.crc4_last_smf)
				crc4_error = true;
			else
				crc4_error = false;
			if (crc4_error != e1i->tx.crc4_error) {
				notify_user(e1i, E1_NTFY_EVT_CRC_ERROR, crc4_error, NULL);
				e1i->tx.crc4_error = crc4_error;
			}
			/* rotate computed CRC4 one further */
			e1i->rx.crc4_last_smf = e1i->rx.crc4;
			e1i->rx.crc4 = crc4itu_init();
			break;
		default:
			break;
		}

		/* check if the remote side reports any CRC errors */
		switch (e1i->rx.frame_nr) {
		case 13:
		case 15:
			crc4_error = false;
			if ((get_ts0_hist(e1i, 0) >> 7) == 0)
				crc4_error = true;
			if (crc4_error != e1i->rx.remote_crc4_error) {
				notify_user(e1i, E1_NTFY_EVT_REMOTE_CRC_ERROR, crc4_error, NULL);
				e1i->rx.remote_crc4_error = crc4_error;
			}
			break;
		}
	}

	e1_aligned_common(e1i);
}

/* FSM State handler */
static void e1_aligned_basic(struct osmo_fsm_inst *fi, uint32_t event, void *data)
{
	struct osmo_e1f_instance *e1i = (struct osmo_e1f_instance *) fi->priv;

	if (frame_alignment_lost(e1i)) {
		osmo_fsm_inst_state_chg(fi, E1_AS_SEARCH_FRAME, 0, 0);
		return;
	}

	e1_aligned_common(e1i);
}

static const struct osmo_fsm_state e1_align_states[] = {
	[E1_AS_SEARCH_FRAME] = {
		.name = "SEARCH_FRAME",
		.in_event_mask = S(E1_AE_RX_TS0),
		.out_state_mask = S(E1_AS_SEARCH_FRAME) |
				  S(E1_AS_SEARCH_CRC_MFRAME) |
				  S(E1_AS_ALIGNED_BASIC),
		.action = e1_align_search_frame,
	},
	[E1_AS_SEARCH_CRC_MFRAME] = {
		.name = "SEARCH_CRC_MFRAME",
		.in_event_mask = S(E1_AE_RX_TS0),
		.out_state_mask = S(E1_AS_SEARCH_FRAME) |
				  S(E1_AS_SEARCH_CRC_MFRAME) |
				  S(E1_AS_ALIGNED_CRC_MFRAME),
		.action = e1_align_search_crc_mframe,
	},
	[E1_AS_ALIGNED_CRC_MFRAME] = {
		.name = "ALIGNED_CRC_MFRAME",
		.in_event_mask = S(E1_AE_RX_TS0),
		.out_state_mask = S(E1_AS_SEARCH_FRAME) |
				  S(E1_AS_SEARCH_CRC_MFRAME) |
				  S(E1_AS_ALIGNED_CRC_MFRAME),
		.action = e1_aligned_crc_mframe,
	},
	[E1_AS_ALIGNED_BASIC] = {
		.name = "ALIGNED_BASIC",
		.in_event_mask = S(E1_AE_RX_TS0),
		.out_state_mask = S(E1_AS_SEARCH_FRAME),
		.action = e1_aligned_basic,
	},
};

static void e1_allstate(struct osmo_fsm_inst *fi, uint32_t event, void *data)
{
	struct osmo_e1f_instance *e1i = (struct osmo_e1f_instance *) fi->priv;

	switch (event) {
	case E1_AE_RESET:
		e1i->rx.num_ts0_in_mframe_search = 0;
		osmo_fsm_inst_state_chg(fi, E1_AS_SEARCH_FRAME, 0, 0);
		break;
	}
}

static struct osmo_fsm e1_align_fsm = {
	.name = "e1-align",
	.states = e1_align_states,
	.num_states = ARRAY_SIZE(e1_align_states),
	.allstate_event_mask = S(E1_AE_RESET),
	.allstate_action = e1_allstate,
	.log_subsys = DLGLOBAL,
	.event_names = e1_align_evt_names,
};

static void align_fsm_reset(struct osmo_e1f_instance *e1i)
{
	osmo_fsm_inst_dispatch(e1i->rx.fi, E1_AE_RESET, NULL);
}

static void e1_rx_hist_add(struct osmo_e1f_instance *e1i, uint8_t inb)
{
	e1i->rx.ts0_history[e1i->rx.frame_nr] = inb;
	if (e1i->rx.ts0_hist_len < 16)
		e1i->rx.ts0_hist_len++;
}

static void e1_rx_ts0(struct osmo_e1f_instance *e1i, uint8_t inb)
{
	/* append just-received byte to the TS0 receive history buffer */
	e1_rx_hist_add(e1i, inb);

	/* notify the FSM that a new TS0 byte was received */
	osmo_fsm_inst_dispatch(e1i->rx.fi, E1_AE_RX_TS0, NULL);

	e1i->rx.frame_nr = (e1i->rx.frame_nr + 1) % 16;
}

static void e1_rx_tsN(struct osmo_e1f_instance_ts *e1t, uint8_t inb)
{
	struct msgb *msg;
	int count, rc;

	if (!e1t->rx.enabled)
		return;

	if (!e1t->rx.msg)
		e1t->rx.msg = msgb_alloc(e1t->rx.granularity, "E1 Rx");
	msg = e1t->rx.msg;
	OSMO_ASSERT(msg);

	switch (e1t->mode) {
	case OSMO_E1F_TS_RAW:
		/* append byte at end of msgb */
		msgb_put_u8(msg, inb);
		/* flush msgb, if full */
		if (msgb_tailroom(msg) <= 0) {
			goto flush;
		}
		break;
	case OSMO_E1F_TS_HDLC_CRC:
		rc = osmo_isdnhdlc_decode(&e1t->rx.hdlc, &inb, 1, &count,
					  msgb_data(msg), msgb_tailroom(msg));
		switch (rc) {
		case -OSMO_HDLC_FRAMING_ERROR:
			fprintf(stdout, "Framing Error\n");
			break;
		case -OSMO_HDLC_CRC_ERROR:
			fprintf(stdout, "CRC Error\n");
			break;
		case -OSMO_HDLC_LENGTH_ERROR:
			fprintf(stdout, "Length Error\n");
			break;
		case 0:
			/* no output yet */
			break;
		default:
			msgb_put(msg, rc);
			goto flush;
		}
		break;
	}

	return;
flush:

	if (!e1t->rx.data_cb)
		msgb_free(msg);
	else
		e1t->rx.data_cb(e1t, msg);
	e1t->rx.msg = NULL;
}

/*! Receive a single E1 frame of 32x8 (=256) bits
 *  \param e1i E1 instance for which the frame was received
 *  \param[in] in_frame caller-provided buffer of 32 octets
 *
 *  The idea is that whoever calls us will already have done the bit-alignment,
 *  i.e. the first bit of TS0 of the frame will be octet-aligned and hence the
 *  entire 256bit buffer is provided as octet-aligned 32bytes in \a in_frame.
 */
int osmo_e1f_rx_frame(struct osmo_e1f_instance *e1i, const uint8_t *in_frame)
{
	int i;

	e1_rx_update_crc4(e1i, in_frame);

	e1_rx_ts0(e1i, in_frame[0]);

	for (i = 1; i < ARRAY_SIZE(e1i->ts); i++) {
		struct osmo_e1f_instance_ts *e1t = &e1i->ts[i];
		e1_rx_tsN(e1t, in_frame[i]);
	}

	return 0;
}

int osmo_e1f_init(void)
{
	return osmo_fsm_register(&e1_align_fsm);
}
