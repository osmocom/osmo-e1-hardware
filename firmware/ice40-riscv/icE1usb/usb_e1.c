/*
 * usb_e1.c
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
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
#include "usb_desc_ids.h"

#include "ice1usb_proto.h"

struct {
	bool running;		/* are we running (transceiving USB data)? */
	int out_bdi;		/* buffer descriptor index for OUT EP */
	int in_bdi;		/* buffer descriptor index for IN EP */
	struct ice1usb_tx_config tx_cfg;
	struct ice1usb_rx_config rx_cfg;
	struct e1_error_count last_err;
} g_usb_e1;

/* default configuration at power-up */
static const struct ice1usb_tx_config tx_cfg_default = {
	.mode		= ICE1USB_TX_MODE_TS0_CRC4_E,
	.timing		= ICE1USB_TX_TIME_SRC_LOCAL,
	.ext_loopback	= ICE1USB_TX_EXT_LOOPBACK_OFF,
	.alarm		= 0,
};

static const struct ice1usb_rx_config rx_cfg_default = {
	.mode		= ICE1USB_RX_MODE_MULTIFRAME,
};


static volatile struct usb_ep *
_get_ep_regs(uint8_t ep)
{
	return (ep & 0x80) ? &usb_ep_regs[ep & 0x1f].in : &usb_ep_regs[ep & 0x1f].out;
}

static void
_usb_fill_feedback_ep(void)
{
	static uint16_t ticks_prev = 0;
	uint16_t ticks;
	uint32_t val = 8192;
	unsigned int level;
	volatile struct usb_ep *ep_regs;

	/* Compute real E1 tick count (with safety against bad values) */
	ticks = e1_tick_read();
	val = (ticks - ticks_prev) & 0xffff;
	ticks_prev = ticks;
	if ((val < 7168) | (val > 9216))
		val = 8192;

	/* Bias depending on TX fifo level */
	level = e1_tx_level();
	if (level < (3 * 16))
		val += 256;
	else if (level > (8 * 16))
		val -= 256;

	/* Prepare buffer */
	ep_regs = _get_ep_regs(USB_EP_E1_FB(0));
	usb_data_write(ep_regs->bd[0].ptr, &val, 4);
	ep_regs->bd[0].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(3);
}


void
usb_e1_run(void)
{
	volatile struct usb_ep *ep_regs;
	int bdi;

	if (!g_usb_e1.running)
		return;

	/* Interrupt endpoint */
	ep_regs = _get_ep_regs(USB_EP_E1_INT(0));

	if ((ep_regs->bd[0].csr & USB_BD_STATE_MSK) != USB_BD_STATE_RDY_DATA) {
		const struct e1_error_count *cur_err = e1_get_error_count();
		if (memcmp(cur_err, &g_usb_e1.last_err, sizeof(*cur_err))) {
			struct ice1usb_irq errmsg = {
				.type = ICE1USB_IRQ_T_ERRCNT,
				.u = {
					.errors = {
						.crc = cur_err->crc,
						.align = cur_err->align,
						.ovfl = cur_err->ovfl,
						.unfl = cur_err->unfl,
						.flags = cur_err->flags,
					}
				}
			};
			printf("E");
			usb_data_write(ep_regs->bd[0].ptr, &errmsg, sizeof(errmsg));
			ep_regs->bd[0].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(sizeof(errmsg));
			g_usb_e1.last_err = *cur_err;
		}
	}

	/* Data IN endpoint */
	ep_regs = _get_ep_regs(USB_EP_E1_IN(0));
	bdi = g_usb_e1.in_bdi;

	while ((ep_regs->bd[bdi].csr & USB_BD_STATE_MSK) != USB_BD_STATE_RDY_DATA)
	{
		uint32_t ptr = ep_regs->bd[bdi].ptr;
		uint32_t hdr;
		unsigned int pos;

		/* Error check */
		if ((ep_regs->bd[bdi].csr & USB_BD_STATE_MSK) == USB_BD_STATE_DONE_ERR)
			puts("Err EP IN\n");

		/* Get some data from E1 */
		int n = e1_rx_level();

		if (n > 64)
			n = 12;
		else if (n > 32)
			n = 10;
		else if (n > 8)
			n = 8;
		else if (!n)
			break;

		n = e1_rx_need_data((ptr >> 2) + 1, n, &pos);

		/* Write header: currently version and pos (mfr/fr number) */
		hdr = (0 << 28) | (pos & 0xff);
		usb_data_write(ptr, &hdr, 4);

		/* Resubmit */
		ep_regs->bd[bdi].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN((n * 32) + 4);

		/* Next BDI */
		bdi ^= 1;
		g_usb_e1.in_bdi = bdi;
	}

	/* Data OUT endpoint */
	ep_regs = _get_ep_regs(USB_EP_E1_OUT(0));
	bdi = g_usb_e1.out_bdi;

	while ((ep_regs->bd[bdi].csr & USB_BD_STATE_MSK) != USB_BD_STATE_RDY_DATA)
	{
		uint32_t ptr = ep_regs->bd[bdi].ptr;
		uint32_t csr = ep_regs->bd[bdi].csr;
		uint32_t hdr;

		/* Error check */
		if ((csr & USB_BD_STATE_MSK) == USB_BD_STATE_DONE_ERR) {
			puts("Err EP OUT\n");
			goto refill;
		}

		/* Grab header */
		usb_data_read(&hdr, ptr, 4);

		/* Empty data into the FIFO */
		int n = ((int)(csr & USB_BD_LEN_MSK) - 6) / 32;
		if (n > 0)
			e1_tx_feed_data((ptr >> 2) + 1, n);

refill:
		/* Refill it */
		ep_regs->bd[bdi].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(388);

		/* Next BDI */
		bdi ^= 1;
		g_usb_e1.out_bdi = bdi;

		static int x = 0;
		if ((x++ & 0xff) == 0xff)
			puts(".");
	}

	/* Feedback endpoint */
	ep_regs = _get_ep_regs(USB_EP_E1_FB(0));

	if ((ep_regs->bd[0].csr & USB_BD_STATE_MSK) != USB_BD_STATE_RDY_DATA)
	{
		_usb_fill_feedback_ep();
	}
}

static enum usb_fnd_resp
_e1_set_conf(const struct usb_conf_desc *conf)
{
	const struct usb_intf_desc *intf;

	printf("e1 set_conf %08x\n", conf);
	if (!conf)
		return USB_FND_SUCCESS;

	intf = usb_desc_find_intf(conf, USB_INTF_E1(0), 0, NULL);
	if (!intf)
		return USB_FND_ERROR;

	printf("e1 set_conf %08x\n", intf);

	usb_ep_boot(intf, USB_EP_E1_IN(0),  true);
	usb_ep_boot(intf, USB_EP_E1_OUT(0), true);
	usb_ep_boot(intf, USB_EP_E1_FB(0),  false);
	usb_ep_boot(intf, USB_EP_E1_INT(0), false);

	return USB_FND_SUCCESS;
}

static void _perform_tx_config(void)
{
	const struct ice1usb_tx_config *cfg = &g_usb_e1.tx_cfg;
	e1_tx_config(   ((cfg->mode & 3) << 1) |
			((cfg->timing & 1) << 3) |
			((cfg->alarm & 1) << 4) |
			((cfg->ext_loopback & 3) << 5) );
}

static void _perform_rx_config(void)
{
	const struct ice1usb_rx_config *cfg = &g_usb_e1.rx_cfg;
	e1_rx_config((cfg->mode << 1));
}

static enum usb_fnd_resp
_e1_set_intf(const struct usb_intf_desc *base, const struct usb_intf_desc *sel)
{
	volatile struct usb_ep *ep_regs;

	/* Validity checks */
	if ((base->bInterfaceClass != 0xff) || (base->bInterfaceSubClass != 0xe1))
		return USB_FND_CONTINUE;

	if (sel->bAlternateSetting > 1)
		return USB_FND_ERROR;

	/* Don't do anything if no change */
	if (g_usb_e1.running == (sel->bAlternateSetting != 0))
		return USB_FND_SUCCESS;

	g_usb_e1.running = (sel->bAlternateSetting != 0);

	/* Reconfigure the endpoints */
	usb_ep_reconf(sel, USB_EP_E1_IN(0));
	usb_ep_reconf(sel, USB_EP_E1_OUT(0));
	usb_ep_reconf(sel, USB_EP_E1_FB(0));
	usb_ep_reconf(sel, USB_EP_E1_INT(0));

	/* Update E1 and USB state */
	switch (g_usb_e1.running) {
	case false:
		/* Disable E1 rx/tx */
		e1_init(0, 0);
		break;

	case true:
		/* Reset and Re-Enable E1 */
		e1_init(0, 0);
		_perform_rx_config();
		_perform_tx_config();

		/* Reset BDI */
		g_usb_e1.in_bdi = 0;
		g_usb_e1.out_bdi = 0;

		/* EP OUT: Queue two buffers */
		ep_regs = _get_ep_regs(USB_EP_E1_FB(0));
		ep_regs->bd[0].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(388);
		ep_regs->bd[1].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(388);

		/* EP Feedback: Pre-fill */
		_usb_fill_feedback_ep();

		break;
	}

	return USB_FND_SUCCESS;
}

static enum usb_fnd_resp
_e1_get_intf(const struct usb_intf_desc *base, uint8_t *alt)
{
	if ((base->bInterfaceClass != 0xff) || (base->bInterfaceSubClass != 0xe1))
		return USB_FND_CONTINUE;

	*alt = g_usb_e1.running ? 1 : 0;

	return USB_FND_SUCCESS;
}

static bool
_set_tx_mode_done(struct usb_xfer *xfer)
{
	const struct ice1usb_tx_config *cfg = (const struct ice1usb_tx_config *) xfer->data;
	printf("set_tx_mode %02x%02x%02x%02x\r\n",
		xfer->data[0], xfer->data[1], xfer->data[2], xfer->data[3]);
	g_usb_e1.tx_cfg = *cfg;
	_perform_tx_config();
	return true;
}

static bool
_set_rx_mode_done(struct usb_xfer *xfer)
{
	const struct ice1usb_rx_config *cfg = (const struct ice1usb_rx_config *) xfer->data;
	printf("set_rx_mode %02x\r\n", xfer->data[0]);
	g_usb_e1.rx_cfg = *cfg;
	_perform_rx_config();
	return true;
}

/* per-interface requests */
static enum usb_fnd_resp
_e1_ctrl_req_intf(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	unsigned int i;

	switch (req->bRequest) {
	case ICE1USB_INTF_GET_CAPABILITIES:
		/* no optional capabilities yet */
		xfer->len = 0;
		break;
	case ICE1USB_INTF_SET_TX_CFG:
		if (req->wLength < sizeof(struct ice1usb_tx_config))
			return USB_FND_ERROR;
		xfer->cb_done = _set_tx_mode_done;
		xfer->cb_ctx = req;
		xfer->len = sizeof(struct ice1usb_tx_config);
		break;
	case ICE1USB_INTF_GET_TX_CFG:
		if (req->wLength < sizeof(struct ice1usb_tx_config))
			return USB_FND_ERROR;
		memcpy(xfer->data, &g_usb_e1.tx_cfg, sizeof(struct ice1usb_tx_config));
		xfer->len = sizeof(struct ice1usb_tx_config);
		break;
	case ICE1USB_INTF_SET_RX_CFG:
		if (req->wLength < sizeof(struct ice1usb_rx_config))
			return USB_FND_ERROR;
		xfer->cb_done = _set_rx_mode_done;
		xfer->cb_ctx = req;
		xfer->len = sizeof(struct ice1usb_rx_config);
		break;
	case ICE1USB_INTF_GET_RX_CFG:
		if (req->wLength < sizeof(struct ice1usb_rx_config))
			return USB_FND_ERROR;
		memcpy(xfer->data, &g_usb_e1.rx_cfg, sizeof(struct ice1usb_rx_config));
		xfer->len = sizeof(struct ice1usb_rx_config);
		break;
	default:
		return USB_FND_ERROR;
	}

	return USB_FND_SUCCESS;
}

/* device-global requests */
static enum usb_fnd_resp
_e1_ctrl_req_dev(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	switch (req->bRequest) {
	case ICE1USB_DEV_GET_CAPABILITIES:
		xfer->data[0] = (1 << ICE1USB_DEV_CAP_GPSDO);
		xfer->len = 1;
		break;
	default:
		return USB_FND_ERROR;
	}

	return USB_FND_SUCCESS;
}


/* USB host issues a control request to us */
static enum usb_fnd_resp
_e1_ctrl_req(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	if (USB_REQ_TYPE(req) != USB_REQ_TYPE_VENDOR)
		return USB_FND_CONTINUE;

	switch (USB_REQ_RCPT(req)) {
	case USB_REQ_RCPT_DEV:
		return _e1_ctrl_req_dev(req, xfer);
	case USB_REQ_RCPT_INTF:
		if (req->wIndex != USB_INTF_E1(0))
			return USB_FND_CONTINUE;
		return _e1_ctrl_req_intf(req, xfer);
	case USB_REQ_RCPT_EP:
	case USB_REQ_RCPT_OTHER:
	default:
		return USB_FND_ERROR;
	}
}


static struct usb_fn_drv _e1_drv = {
	.set_conf	= _e1_set_conf,
        .set_intf       = _e1_set_intf,
        .get_intf       = _e1_get_intf,
	.ctrl_req	= _e1_ctrl_req,
};

void
usb_e1_init(void)
{
	memset(&g_usb_e1, 0x00, sizeof(g_usb_e1));
	g_usb_e1.tx_cfg = tx_cfg_default;
	g_usb_e1.rx_cfg = rx_cfg_default;
	usb_register_function_driver(&_e1_drv);
}
