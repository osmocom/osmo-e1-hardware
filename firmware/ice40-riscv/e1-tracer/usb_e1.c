/*
 * usb_e1.c
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * Copyright (C) 2022  Harald Welte <laforge@osmocom.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <no2usb/usb_hw.h>
#include <no2usb/usb_priv.h>

#include "console.h"
#include "e1.h"
#include "misc.h"

struct {
	bool running[2];
	int in_bdi[2];
} g_usb_e1;


/* Hack */
unsigned int e1_rx_need_data(int chan, unsigned int usb_addr, unsigned int max_len, unsigned int *pos);
unsigned int e1_rx_level(int chan);
uint8_t e1_get_pending_flags(int chan);
/* ---- */


void
usb_e1_run(void)
{
	int chan;
	int bdi;

	/* EP[1-2] IN */
	for (chan=0; chan<2; chan++)
	{
		if (!g_usb_e1.running[chan])
			continue;

		bdi = g_usb_e1.in_bdi[chan];

		while ((usb_ep_regs[1+chan].in.bd[bdi].csr & USB_BD_STATE_MSK) != USB_BD_STATE_RDY_DATA)
		{
			uint32_t ptr = usb_ep_regs[1+chan].in.bd[bdi].ptr;
			uint32_t hdr;
			unsigned int pos;

			/* Error check */
			if ((usb_ep_regs[1+chan].in.bd[bdi].csr & USB_BD_STATE_MSK) == USB_BD_STATE_DONE_ERR)
				printf("Err EP%d IN\n", 1+chan);

			/* Get some data from E1 */
			int n = e1_rx_level(chan);

//			if (n > 64)
//				n = 12;
//			else if (n > 32)
//				n = 10;
//			else if (n > 8)
//				n = 8;
			if (n > 12)
				n = 12;
			else if (!n)
				break;

			n = e1_rx_need_data(chan, (ptr >> 2) + 1, n, &pos);

			/* Write header */
				/* [31:12] (reserved) */
				/* [11:10] CRC results (first new multiframe present in packet)  */
				/* [ 9: 8] CRC results (second new multiframe present in packet) */
				/* [ 7: 5] Multiframe sequence number (first frame of packet)    */
				/* [ 4: 0] Position in multi-frame    (first frame of packet)    */
			hdr = (pos & 0xff) | (e1_get_pending_flags(chan) << 24);
			usb_data_write(ptr, &hdr, 4);
			usb_ep_regs[1+chan].in.bd[bdi].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN((n * 32) + 4);

			/* Next BDI */
			bdi ^= 1;
			g_usb_e1.in_bdi[chan] = bdi;
		}
	}
}

static const struct usb_intf_desc *
_find_intf(const struct usb_conf_desc *conf, uint8_t idx)
{
	const struct usb_intf_desc *intf = NULL;
	const void *sod, *eod;

	if (!conf)
		return NULL;

	sod = conf;
	eod = sod + conf->wTotalLength;

	while (1) {
		sod = usb_desc_find(sod, eod, USB_DT_INTF);
		if (!sod)
			break;

		intf = (void*)sod;
		if (intf->bInterfaceNumber == idx)
			return intf;

		sod = usb_desc_next(sod);
	}

	return NULL;
}

static const struct usb_conf_desc *last_conf;

enum usb_fnd_resp
_e1_set_conf(const struct usb_conf_desc *conf)
{
	const struct usb_intf_desc *intf;

	printf("e1 set_conf %08x\n", conf);

	last_conf = conf;
	if (!conf)
		return USB_FND_SUCCESS;

	intf = _find_intf(conf, 0);
	if (!intf)
		return USB_FND_ERROR;

	printf("e1 set_conf %08x\n", intf);

	usb_ep_boot(intf, 0x81, true);
	usb_ep_boot(intf, 0x82, true);

	return USB_FND_SUCCESS;
}

static void
disable_chan(int chan)
{
	/* Already stopped ? */
	if (!g_usb_e1.running[chan])
		return;

	/* Update state */
	g_usb_e1.running[chan] = false;

	/* Stop E1 */
	e1_stop(chan);

	/* Disable end-points */
	usb_ep_regs[chan+1].in.status = 0;
}

static void
enable_chan(int chan)
{
	/* Already running ? */
	if (g_usb_e1.running[chan])
		return;

	/* Update state */
	g_usb_e1.running[chan] = true;

	/* Reset buffer pointers */
	g_usb_e1.in_bdi[chan] = 0;

	/* Configure EP1 IN / EP2 IN */
	usb_ep_regs[chan+1].in.status = USB_EP_TYPE_ISOC | USB_EP_BD_DUAL;	/* Type=Isochronous, dual buffered */

	/* EP1 IN: Prepare two buffers */
	usb_ep_regs[chan+1].in.bd[0].ptr = 256 + (chan * 2 + 0) * 388;
	usb_ep_regs[chan+1].in.bd[0].csr = 0;

	usb_ep_regs[chan+1].in.bd[1].ptr = 256 + (chan * 2 + 1) * 388;
	usb_ep_regs[chan+1].in.bd[1].csr = 0;

	/* Start E1 */
	e1_start(chan);
}

enum usb_fnd_resp
_e1_set_intf(const struct usb_intf_desc *base, const struct usb_intf_desc *sel)
{
	if (!last_conf || last_conf->bConfigurationValue == 1) {
		/* Legacy Configuration */

		if (base->bInterfaceNumber != 0)
			return USB_FND_CONTINUE;

		if (sel->bAlternateSetting == 0) {
			disable_chan(0);
			disable_chan(1);
		} else if (sel->bAlternateSetting == 1) {
			enable_chan(0);
			enable_chan(1);
		} else {
			/* Unknown */
			return USB_FND_ERROR;
		}
	} else if (last_conf && last_conf->bConfigurationValue == 2) {
		/* e1d compatible configuration */

		switch (base->bInterfaceNumber) {
		case 0:
		case 1:
			switch (sel->bAlternateSetting) {
			case 0:
				disable_chan(base->bInterfaceNumber);
				break;
			case 1:
				enable_chan(base->bInterfaceNumber);
				break;
			default:
				/* Unknown */
				return USB_FND_ERROR;
			}
			break;
		default:
			return USB_FND_CONTINUE;
		}
	} else {
		return USB_FND_ERROR;
	}

	return USB_FND_SUCCESS;
}

enum usb_fnd_resp
_e1_get_intf(const struct usb_intf_desc *base, uint8_t *alt)
{
	if (!last_conf || last_conf->bConfigurationValue == 1) {
		/* Legacy configuration */
		if (base->bInterfaceNumber != 0)
			return USB_FND_CONTINUE;

		*alt = g_usb_e1.running[0] && g_usb_e1.running[1] ? 1 : 0;
	} else if (last_conf && last_conf->bConfigurationValue == 2) {
		/* e1d compatible configuration */
		switch (base->bInterfaceNumber) {
		case 0:
		case 1:
			*alt = g_usb_e1.running[base->bInterfaceNumber] ? 1 : 0;
			break;
		default:
			return USB_FND_CONTINUE;
		}
	} else
		return USB_FND_CONTINUE;

	return USB_FND_SUCCESS;
}

static struct usb_fn_drv _e1_drv = {
	.set_conf	= _e1_set_conf,
        .set_intf       = _e1_set_intf,
        .get_intf       = _e1_get_intf,
};

void
usb_e1_init(void)
{
	/* Clear state */
	memset(&g_usb_e1, 0x00, sizeof(g_usb_e1));

	/* Install driver */
	usb_register_function_driver(&_e1_drv);
}
