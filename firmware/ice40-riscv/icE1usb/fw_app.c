/*
 * fw_app.c
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <no2usb/usb.h>
#include <no2usb/usb_dfu_rt.h>

#include "console.h"
#include "e1.h"
#include "gps.h"
#include "gpsdo.h"
#include "led.h"
#include "misc.h"
#include "mini-printf.h"
#include "spi.h"
#include "usb_dev.h"
#include "usb_e1.h"
#include "usb_gps.h"
#include "utils.h"


extern const struct usb_stack_descriptors app_stack_desc;

static void
serial_no_init()
{
	uint8_t buf[8];
	char *id, *desc;
	int i;

	flash_manuf_id(buf);
	printf("Flash Manufacturer : %s\n", hexstr(buf, 3, true));

	flash_unique_id(buf);
	printf("Flash Unique ID    : %s\n", hexstr(buf, 8, true));

	/* Overwrite descriptor string */
		/* In theory in rodata ... but nothing is ro here */
	id = hexstr(buf, 8, false);
	desc = (char*)app_stack_desc.str[1];
	for (i=0; i<16; i++)
		desc[2 + (i << 1)] = id[i];
}

static void
boot_dfu(void)
{
	/* Force re-enumeration */
	usb_disconnect();

	/* Boot firmware */
	reboot(1);
}

void
usb_dfu_rt_cb_reboot(void)
{
        boot_dfu();
}


void main()
{
	int cmd = 0;

	/* Init console IO */
	console_init();
	printf("\n\nBooting %s\n", fw_build_str);

	/* LED */
	led_init();

	/* SPI */
	spi_init();
	serial_no_init();

	/* Enable LED now that we're done with SPI */
	e1_led_set(true, 0x00);

	/* Setup E1 Vref */
	int d = 25;
#if defined(BOARD_ICE1USB_PROTO_ICEBREAKER) || defined(BOARD_ICE1USB_PROTO_BITSY)
	pdm_set(PDM_E1_CT, true, 128, false);
	pdm_set(PDM_E1_P,  true, 128 - d, false);
	pdm_set(PDM_E1_N,  true, 128 + d, false);
#else
	pdm_set(PDM_E1_RX0,  true, 128 + d, false);
	pdm_set(PDM_E1_RX1,  true, 128 + d, false);
#endif

	/* GPS init */
	gps_init();
#if defined(BOARD_ICE1USB_PROTO_ICEBREAKER) || defined(BOARD_ICE1USB_PROTO_BITSY)
	gpsdo_init(VCTXO_TAITIEN_VT40);
#else
	gpsdo_init(VCTXO_SITIME_SIT3808_E);
#endif

	/* Enable USB directly */
	usb_init(&app_stack_desc);
	usb_dev_init();
	usb_dfu_rt_init();
	usb_e1_init();
	usb_gps_init();

	/* Start */
	led_state(true);
	usb_connect();

	/* Main loop */
	while (1)
	{
		/* Prompt ? */
		if (cmd >= 0)
			printf("Command> ");

		/* Poll for command */
		cmd = getchar_nowait();

		if (cmd >= 0) {
			if (cmd > 32 && cmd < 127)
				putchar(cmd);
			putchar('\r');
			putchar('\n');

			switch (cmd)
			{
			case 'b':
				boot_dfu();
				break;
			case 'p':
				panic("Test panic");
				break;
			case 'q':
				e1_debug_print(0, false);
				break;
			case 'Q':
				e1_debug_print(0, true);
				break;
			case 'w':
				e1_debug_print(1, false);
				break;
			case 'W':
				e1_debug_print(1, true);
				break;
			case 'c':
				usb_connect();
				break;
			case 'd':
				usb_disconnect();
				break;
			case 'u':
				usb_debug_print();
				break;
			default:
				break;
			}
		}

		/* USB poll */
		usb_poll();

		/* E1 poll */
		usb_e1_poll();

		/* GPS poll */
		gps_poll();
		gpsdo_poll();
		usb_gps_poll();
	}
}
