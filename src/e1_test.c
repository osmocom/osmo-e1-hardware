#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <osmocom/core/utils.h>
#include <osmocom/core/logging.h>
#include <osmocom/core/application.h>
#include <osmocom/gsm/gsm_utils.h>

#include "osmo_e1.h"

static struct osmo_e1_instance inst;
static struct log_info log_info = {};

/* pull data out of the transmitter and print hexdumps */
static void pull_and_print(struct osmo_e1_instance *e1i)
{
	uint8_t buf[32];
	osmo_e1_pull_tx_frame(e1i, buf);
	printf("%s\n", osmo_hexdump(buf, sizeof(buf)));
}

static void data_cb(struct osmo_e1_instance_ts *e1t, struct msgb *msg)
{
	printf("Rx TS %u: %s\n", e1t->ts_nr, msgb_hexdump(msg));
	msgb_free(msg);
}

static void notify_cb(struct osmo_e1_instance *e1i, enum osmo_e1_notify_event evt,
			bool present, void *data)
{
	printf("NOTIFY: %s %s\n", osmo_e1_notify_event_name(evt), present ? "PRESENT" : "ABSENT");
}

/* feed some random data into the E1 instance */
static void tc_rx_random()
{
	uint8_t buf[32];
	int i;

	for (i = 0; i < 200; i++) {
		osmo_get_rand_id(buf, sizeof(buf));
		osmo_e1_rx_frame(&inst, buf);
	}
}

static void tc_rx_align_basic()
{
	uint8_t buf[32];
	int i;

	for (i = 0; i < 80; i++) {
		memset(buf, 0xff, sizeof(buf));
		switch (i %2) {
		case 0:
			buf[0] = 0x9B;
			break;
		case 1:
			buf[0] = 0x40;
			break;
		}
		osmo_e1_rx_frame(&inst, buf);
	}
}

static void tc_rx_align_mframe()
{
	uint8_t buf[32];
	int i;

	for (i = 0; i < 80; i++) {
		memset(buf, 0xff, sizeof(buf));
		switch (i % 16) {
		case 0:
		case 2:
		case 4:
		case 6:
		case 8:
		case 10:
		case 12:
		case 14:
			buf[0] = 0x9B;
			break;
		case 1:
		case 3:
		case 7:
		case 13:
		case 15:
			buf[0] = 0x40;
			break;
		case 5:
		case 9:
		case 11:
			buf[0] = 0xc0;
			break;
		}
		osmo_e1_rx_frame(&inst, buf);
	}
}


static void tc_tx_idle()
{
	int i;
	for (i = 0; i < 20; i++) {
		pull_and_print(&inst);
	}
}

int main(int argc, char **argv)
{
	int i;

	osmo_init_logging2(NULL, &log_info);
	osmo_e1_init();

	osmo_e1_instance_init(&inst, "e1_test", &notify_cb, true, NULL);
	for (i = 1; i < 32; i++) {
		struct osmo_e1_instance_ts *e1t = osmo_e1_instance_ts(&inst, i);
		osmo_e1_ts_config(e1t, &data_cb, 40, true, OSMO_E1_TS_RAW);
	}

	printf("\nRx Random...\n");
	osmo_e1_instance_reset(&inst);
	tc_rx_random();

	printf("\nAlign (Basic)...\n");
	osmo_e1_instance_reset(&inst);
	tc_rx_align_basic();

	printf("\nAlign (Mframe)...\n");
	osmo_e1_instance_reset(&inst);
	tc_rx_align_mframe();

	printf("\nTX Idle...\n");
	osmo_e1_instance_reset(&inst);
	tc_tx_idle();

}
