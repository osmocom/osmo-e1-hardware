#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <osmocom/core/utils.h>
#include <osmocom/core/logging.h>
#include <osmocom/core/application.h>
#include <osmocom/gsm/gsm_utils.h>

#include "osmo_e1f.h"

static struct osmo_e1f_instance inst;
static struct log_info log_info = {};

static void data_cb(struct osmo_e1f_instance_ts *e1t, struct msgb *msg)
{
	printf("Rx TS %02u: %s\n", e1t->ts_nr, msgb_hexdump(msg));
	msgb_free(msg);
}

static void notify_cb(struct osmo_e1f_instance *e1i, enum osmo_e1f_notify_event evt,
			bool present, void *data)
{
	fprintf(stdout, "NOTIFY: %s %s\n", osmo_e1f_notify_event_name(evt), present ? "PRESENT" : "ABSENT");
}

static void read_file(const char *fname)
{
	int fd;

	fd = open(fname, O_RDONLY);
	if (fd < 0)
		exit(23);
	while (1) {
		int rc;
		uint8_t buf[32];

		rc = read(fd, buf, sizeof(buf));
		if (rc <= 0)
			return;
		if (rc < sizeof(buf))
			exit(24);
		//printf("FRAME: %s\n", osmo_hexdump(buf, sizeof(buf)));
		osmo_e1f_rx_frame(&inst, buf);
	}
}

int main(int argc, char **argv)
{
	int i;

	osmo_init_logging2(NULL, &log_info);
	osmo_e1f_init();

	osmo_e1f_instance_init(&inst, "e1_test", &notify_cb, true, NULL);
	for (i = 1; i < 32; i++) {
		struct osmo_e1f_instance_ts *e1t = osmo_e1f_instance_ts(&inst, i);
		enum osmo_e1f_ts_mode mode;
		bool enable;
		switch (i) {
		case 2:
			mode = OSMO_E1F_TS_HDLC_CRC;
			enable = true;
			break;
		case 5:
		case 6:
		case 7:
		case 8:
		default:
			mode = OSMO_E1F_TS_RAW;
			enable = false;
			break;
		}
		osmo_e1f_ts_config(e1t, &data_cb, 64, enable, mode);
	}

	read_file("Insite_to_Racal_E1.bin");
}
