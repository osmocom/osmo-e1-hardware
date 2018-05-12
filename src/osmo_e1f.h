#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <osmocom/core/msgb.h>
#include <osmocom/core/linuxlist.h>
#include <osmocom/core/fsm.h>
#include <osmocom/core/isdnhdlc.h>

struct osmo_e1f_tx_state {
	bool remote_alarm;
	bool crc4_error;
	/* lower 5 bits: Sa4..Sa8 */
	uint8_t sa4_sa8;
	/* frame number 0..15 */
	uint8_t frame_nr;
	uint8_t crc4_last_smf;
	uint8_t crc4;
};

struct osmo_e1f_rx_state {
	uint8_t frame_nr;
	/* history of rceived TS0 octets */
	uint8_t ts0_history[16];
	uint8_t ts0_hist_len;
	/* was a remote alarm received? */
	bool remote_alarm;
	bool remote_crc4_error;
	/* number of TS0 bytes received since entering CRC mframe search */
	uint8_t num_ts0_in_mframe_search;
	struct osmo_fsm_inst *fi;
	/* computed CRC4 */
	uint8_t crc4_last_smf;
	uint8_t crc4;
};

enum osmo_e1f_notify_event {
	E1_NTFY_EVT_ALIGN_FRAME,
	E1_NTFY_EVT_ALIGN_CRC_MFRAME,
	E1_NTFY_EVT_CRC_ERROR,
	E1_NTFY_EVT_REMOTE_CRC_ERROR,
	E1_NTFY_EVT_REMOTE_ALARM,
};

enum osmo_e1f_ts_mode {
       OSMO_E1F_TS_RAW,
       OSMO_E1F_TS_HDLC_CRC,
};

struct osmo_e1f_instance_ts;
struct osmo_e1f_instance;
typedef void (*e1_data_cb)(struct osmo_e1f_instance_ts *ts, struct msgb *msg);
typedef void (*e1_notify_cb)(struct osmo_e1f_instance *e1i, enum osmo_e1f_notify_event evt,
			     bool present, void *data);

struct osmo_e1f_instance_ts {
	/* timeslot number */
	uint8_t ts_nr;
	/* mode in which we operate (RAW/HDLC) */
	enum osmo_e1f_ts_mode mode;
	/* back-pointer to e1 instance */
	struct osmo_e1f_instance *inst;
	struct {
		/* optional HDLC encoder state */
		struct osmo_isdnhdlc_vars hdlc;
		/* queue of pending to-be-transmitted messages */
		struct llist_head queue;
		unsigned long underruns;
	} tx;
	struct {
		/* optional HDLC decoder state */
		struct osmo_isdnhdlc_vars hdlc;
		bool enabled;
		/* how many bytes to buffer before calling call-back */
		unsigned int granularity;
		/* current receive buffer */
		struct msgb *msg;
		e1_data_cb data_cb;
		/* private data, relevant to user */
		void *priv;
	} rx;
};

struct osmo_e1f_instance {
	/* list; currently not used yet */
	struct llist_head list;

	/* is CRC4 generation + parsing enabled? */
	bool crc4_enabled;
	/* notification call-back function */
	e1_notify_cb notify_cb;

	/* Rx + Tx related state */
	struct osmo_e1f_tx_state tx;
	struct osmo_e1f_rx_state rx;

	/* our 32 timeslots (only 1..32 are used) */
	struct osmo_e1f_instance_ts ts[32];

	/* private data, relevant to user */
	void *priv;
};

extern const struct value_string osmo_e1f_notifv_evt_names[];

static inline const char *osmo_e1f_notify_event_name(enum osmo_e1f_notify_event evt) {
	return get_value_string(osmo_e1f_notifv_evt_names, evt);
}

int osmo_e1f_init(void);
struct osmo_e1f_instance_ts *osmo_e1f_instance_ts(struct osmo_e1f_instance *e1i, uint8_t ts_nr);
int osmo_e1f_instance_init(struct osmo_e1f_instance *e1i, const char *name, e1_notify_cb cb,
			  bool crc4_enabled, void *priv);
void osmo_e1f_instance_reset(struct osmo_e1f_instance *e1i);
int osmo_e1f_ts_config(struct osmo_e1f_instance_ts *e1t, e1_data_cb cb, unsigned int granularity,
		      bool enable, enum osmo_e1f_ts_mode mode);
void osmo_e1f_ts_reset(struct osmo_e1f_instance_ts *e1t);


void osmo_e1f_ts_enqueue(struct osmo_e1f_instance_ts *e1t, struct msgb *msg);
int osmo_e1f_pull_tx_frame(struct osmo_e1f_instance *e1i, uint8_t *out_frame);
int osmo_e1f_rx_frame(struct osmo_e1f_instance *e1i, const uint8_t *in_frame);
