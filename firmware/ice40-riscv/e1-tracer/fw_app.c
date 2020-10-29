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
#include "led.h"
#include "misc.h"
#include "mini-printf.h"
#include "spi.h"
#include "usb_e1.h"
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



static uint8_t
liu_read_reg(unsigned chan, uint8_t reg)
{
        uint8_t cmd = reg | 0x20;
	uint8_t rv;
        struct spi_xfer_chunk xfer[2] = {
                { .data = (void*)&cmd, .len = 1, .read = false, .write = true,  },
                { .data = (void*)&rv,  .len = 1, .read = true,  .write = false, },
        };
        spi_xfer(SPI_CS_LIU(chan), xfer, 2);
	return rv;
}

static void
liu_write_reg(unsigned chan, uint8_t reg, uint8_t val)
{
        uint8_t cmd[2] = { reg, val };
        struct spi_xfer_chunk xfer[2] = {
                { .data = (void*)cmd, .len = 2, .read = false, .write = true,  },
	};
        spi_xfer(SPI_CS_LIU(chan), xfer, 1);
}



#define USB_RT_LIU_REG_WRITE	((1 << 8) | 0x42)
#define USB_RT_LIU_REG_READ	((2 << 8) | 0xc2)


static bool
_liu_reg_write_done_cb(struct usb_xfer *xfer)
{
	struct usb_ctrl_req *req = xfer->cb_ctx;
	unsigned chan = req->wIndex - 0x81;

	liu_write_reg(chan, req->wValue, xfer->data[0]);

	return true;
}

static enum usb_fnd_resp
_liu_ctrl_req(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	unsigned chan;

	/* If this a vendor request for the E1 endpoints */
	if (USB_REQ_RCPT(req) != USB_REQ_RCPT_EP)
		return USB_FND_CONTINUE;

	if (req->wIndex < 0x81 || req->wIndex > 0x82)
		return USB_FND_CONTINUE;

	if (USB_REQ_TYPE(req) != USB_REQ_TYPE_VENDOR)
		return USB_FND_CONTINUE;

	chan = req->wIndex - 0x81;

	/* */
	switch (req->wRequestAndType)
	{
	case USB_RT_LIU_REG_WRITE:
		xfer->len = 1;
		xfer->cb_done = _liu_reg_write_done_cb;
		xfer->cb_ctx = req;
		break;

	case USB_RT_LIU_REG_READ:
		xfer->len = 1;
		xfer->data[0] = liu_read_reg(chan, req->wValue);
		break;

	default:
		goto error;
	}

	return USB_FND_SUCCESS;

error:
	return USB_FND_ERROR;
}

static struct usb_fn_drv _liu_drv = {
	.ctrl_req = _liu_ctrl_req,
};



void main()
{
	int cmd = 0;

	/* Init console IO */
	console_init();
	puts("Booting App image..\n");

	/* LED */
	led_init();
	led_state(true);

	/* SPI */
	spi_init();

	puts("XXX\n");

	/* Enable USB directly */
	serial_no_init();
	usb_init(&app_stack_desc);
	usb_dfu_rt_init();
	usb_register_function_driver(&_liu_drv);
	usb_e1_init();

	usb_connect();
	e1_init();

	/* Main loop */
	while (1)
	{
		/* Prompt ? */
		if (cmd >= 0)
			printf("Command> ");

		/* Poll for command */
		cmd = getchar_nowait();

		if (cmd >= 0) {
			if (cmd > 32 && cmd < 127) {
				putchar(cmd);
				putchar('\r');
				putchar('\n');
			}

			switch (cmd)
			{
			case 'u':
				usb_debug_print();
				break;
			case 'e':
				e1_debug_print(false);
				break;
			case 'E':
				e1_debug_print(true);
				break;
			default:
				break;
			}
		}

		/* USB poll */
		usb_poll();

		/* E1 poll */
		e1_poll();
		usb_e1_run();
	}
}
