/*
 * usb_gps.c
 *
 * Copyright (C) 2019-2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <no2usb/usb.h>
#include <no2usb/usb_hw.h>
#include <no2usb/usb_priv.h>

#include <no2usb/usb_proto.h>
#include <no2usb/usb_cdc_proto.h>

#include "console.h"
#include "misc.h"
#include "usb_desc_ids.h"


#define BUF_SIZE_LOG	9
#define BUF_SIZE	(1 << BUF_SIZE_LOG)
#define BUF_MASK	(BUF_SIZE - 1)

static struct {
	/* State */
	bool active;

	/* Buffer */
	struct {
		unsigned int wr;
		unsigned int rd;
		char data[BUF_SIZE] __attribute__((aligned(4)));
	} buf;

#ifdef GPS_PPS_ON_CD
	/* PPS tracking */
	struct {
		uint32_t last;
		bool set;
	} pps;
#endif
} g_usb_gps;


static void
_usb_gps_set_active(bool active)
{
	/* Save new state */
	g_usb_gps.active = active;

	/* Reset FIFO if disabled */
	if (!active)
		g_usb_gps.buf.wr = g_usb_gps.buf.rd = 0;
}

static int
_usb_gps_fill_packet(unsigned int dst_ofs)
{
	unsigned int len;

	/* Len available, limited to 64 */
	len = (g_usb_gps.buf.wr - g_usb_gps.buf.rd) & BUF_MASK;

	if (len > 64)
		len = 64;

	/* Copy block */
	usb_data_write(dst_ofs, &g_usb_gps.buf.data[g_usb_gps.buf.rd], (len + 3) & ~3);

	/* Increment read pointer */
	g_usb_gps.buf.rd = (g_usb_gps.buf.rd + len) & BUF_MASK;

	/* If length was not multiple of 4, we emptied the FIFO,
	 * so we reset it to 0 so we're aligned again */
	if (len & 3)
		g_usb_gps.buf.wr = g_usb_gps.buf.rd = 0;

	return len;
}


void
usb_gps_puts(const char *str)
{
	unsigned int nxt;
	char c;

	if (!g_usb_gps.active)
		return;

	while ((c = *str++) != '\0')
	{
		/* Next write pointer pos and full check */
		nxt = (g_usb_gps.buf.wr + 1) & BUF_MASK;
		if (nxt == g_usb_gps.buf.rd)
			/* If overflow, we keep the latest content ... */
			g_usb_gps.buf.rd = (g_usb_gps.buf.rd + 1) & BUF_MASK;

		/* Write data */
		g_usb_gps.buf.data[g_usb_gps.buf.wr] = c;

		/* Update write pointer */
		g_usb_gps.buf.wr = nxt;
	}
}

void
usb_gps_poll(void)
{
	volatile struct usb_ep *ep_regs;

	/* OUT EP: We accept data and throw it away */
	ep_regs = &usb_ep_regs[USB_EP_GPS_CDC_OUT & 0x1f].out;
	ep_regs->bd[0].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(64);

	/* IN EP: Send whatever is queued */
	ep_regs = &usb_ep_regs[USB_EP_GPS_CDC_OUT & 0x1f].in;

	if ((ep_regs->bd[0].csr & USB_BD_STATE_MSK) != USB_BD_STATE_RDY_DATA)
	{
		int len = _usb_gps_fill_packet(ep_regs->bd[0].ptr);

		if (len)
			ep_regs->bd[0].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(len);
		else
			ep_regs->bd[0].csr = 0;
	}

#ifdef GPS_PPS_ON_CD
	/* IN EP CTL: Send PPS pulse */
	ep_regs = &usb_ep_regs[USB_EP_GPS_CDC_CTL & 0x1f].in;

	if ((ep_regs->bd[0].csr & USB_BD_STATE_MSK) != USB_BD_STATE_RDY_DATA)
	{
		/* Default request */
			/* Put as static to work around gcc aliasing bug ... */
		static struct usb_cdc_notif_serial_state notif = {
			.hdr = {
				.bmRequestType = USB_REQ_READ | USB_REQ_TYPE_CLASS | USB_REQ_RCPT_INTF,
				.bRequest      = USB_NOTIF_CDC_SERIAL_STATE,
				.wValue        = 0,
				.wIndex        = USB_INTF_GPS_CDC_CTL,
				.wLength       = 2
			},
			.bits = 0x00
		};

		/* Check if PPS occurred */
		uint32_t pps_now = time_pps_read();

		if (pps_now != g_usb_gps.pps.last)
		{
			/* Update last */
			g_usb_gps.pps.last = pps_now;

			/* Queue CD Set */
			notif.bits = 1;
			usb_data_write(ep_regs->bd[0].ptr, &notif, 12);
			ep_regs->bd[0].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(10);

			/* Need to clear in the future */
			g_usb_gps.pps.set = true;
		}
		else if (g_usb_gps.pps.set)
		{
			/* Queue CD Clear */
			notif.bits = 0;
			usb_data_write(ep_regs->bd[0].ptr, &notif, 12);
			ep_regs->bd[0].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(10);

			/* Cleared */
			g_usb_gps.pps.set = false;
		}
	}
#endif
}


static enum usb_fnd_resp
_cdc_set_conf(const struct usb_conf_desc *conf)
{
	const struct usb_intf_desc *intf;

	if (!conf)
		return USB_FND_SUCCESS;

	/* Configure control interface */
	intf = usb_desc_find_intf(conf, USB_INTF_GPS_CDC_CTL, 0, NULL);
	if (!intf)
		return USB_FND_ERROR;

	usb_ep_boot(intf, USB_EP_GPS_CDC_CTL, false);

	/* Configure data interface */
	intf = usb_desc_find_intf(conf, USB_INTF_GPS_CDC_DATA, 0, NULL);
	if (!intf)
		return USB_FND_ERROR;

	usb_ep_boot(intf, USB_EP_GPS_CDC_OUT, false);
	usb_ep_boot(intf, USB_EP_GPS_CDC_IN,  false);

	return USB_FND_SUCCESS;
}

static enum usb_fnd_resp
_cdc_ctrl_req(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	static const struct usb_cdc_line_coding linecoding = {
		.dwDTERate   = 115200,
		.bCharFormat = 2,
		.bParityType = 0,
		.bDataBits   = 8,
	};

	/* Check this is handled here */
	if (USB_REQ_TYPE_RCPT(req) != (USB_REQ_TYPE_CLASS | USB_REQ_RCPT_INTF))
		return USB_FND_CONTINUE;

	if (req->wIndex != USB_INTF_GPS_CDC_CTL)
		return USB_FND_CONTINUE;

	/* Process request */
	switch (req->bRequest) {
	case USB_REQ_CDC_SEND_ENCAPSULATED_COMMAND:
		/* We don't support any, so just accept and don't care */
		return USB_FND_SUCCESS;

	case USB_REQ_CDC_GET_ENCAPSULATED_RESPONSE:
		/* Never anything to return */
		xfer->len = 0;
		return USB_FND_SUCCESS;

	case USB_REQ_CDC_SET_LINE_CODING:
		/* We only support 1 config, doesn't matter what the hosts sends */
		return USB_FND_SUCCESS;

	case USB_REQ_CDC_GET_LINE_CODING:
		/* We only support 1 config, send that back */
		xfer->data = (void*)&linecoding;
		xfer->len  = sizeof(linecoding);
		return USB_FND_ERROR;

	case USB_REQ_CDC_SET_CONTROL_LINE_STATE:
		/* Enable if DTR is set */
		_usb_gps_set_active((req->wValue & 1) != 0);
		return USB_FND_SUCCESS;
	}

	/* Anything else is not handled */
	return USB_FND_ERROR;
}


static struct usb_fn_drv _cdc_drv = {
	.set_conf	= _cdc_set_conf,
	.ctrl_req	= _cdc_ctrl_req,
};

void
usb_gps_init(void)
{
	memset(&g_usb_gps, 0x00, sizeof(g_usb_gps));
	usb_register_function_driver(&_cdc_drv);
}
