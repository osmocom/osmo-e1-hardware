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
#include "utils.h"

#include "ice1usb_proto.h"

struct usb_e1_state {
	bool running;		/* are we running (transceiving USB data)? */
	int out_bdi;		/* buffer descriptor index for OUT EP */
	int in_bdi;		/* buffer descriptor index for IN EP */
	struct ice1usb_tx_config tx_cfg;
	struct ice1usb_rx_config rx_cfg;
	struct e1_error_count last_err;
};

static struct usb_e1_state g_usb_e1[2];

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

static struct usb_e1_state *
_get_state(int port)
{
	if ((port < 0) || (port > 1))
		panic("_get_state invalid port %d", port);
	return &g_usb_e1[port];
}

static int
_ifnum2port(uint8_t bInterfaceNumber)
{
	switch (bInterfaceNumber) {
	case USB_INTF_E1(0): return 0;
	case USB_INTF_E1(1): return 1;
	default:
		/* Don't panic since this will be handled as USB STALL */
		return -1;
	}
}


static void
_usb_fill_feedback_ep(int port)
{
	volatile struct usb_ep *ep_regs = _get_ep_regs(USB_EP_E1_FB(port));

	/* Always ensure we're ready to send */
	if ((ep_regs->bd[0].csr & USB_BD_STATE_MSK) != USB_BD_STATE_RDY_DATA)
	{
		uint32_t val = 8192;

		/* Add instant bias depending on TX fifo level */
		unsigned int level = e1_tx_level(port);

		if (level < (4 * 16))
			val += 256;
		else if (level > (6 * 16))
			val -= 256;

		/* Fill buffer and submit it */
		usb_data_write(ep_regs->bd[0].ptr, &val, 4);
		ep_regs->bd[0].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(3);
	}
}

static void
_fill_irq_err(struct ice1usb_irq_err *out, const struct e1_error_count *cur_err)
{
	out->crc = cur_err->crc;
	out->align = cur_err->align;
	out->ovfl = cur_err->ovfl;
	out->unfl = cur_err->unfl;
	out->flags = cur_err->flags;
}

static void
_usb_e1_run(int port)
{
	struct usb_e1_state *usb_e1 = _get_state(port);
	volatile struct usb_ep *ep_regs;
	int bdi;

	if (!usb_e1->running)
		return;

	/* Interrupt endpoint */
	ep_regs = _get_ep_regs(USB_EP_E1_INT(port));

	if ((ep_regs->bd[0].csr & USB_BD_STATE_MSK) != USB_BD_STATE_RDY_DATA) {
		const struct e1_error_count *cur_err = e1_get_error_count(port);
		if (memcmp(cur_err, &usb_e1->last_err, sizeof(*cur_err))) {
			struct ice1usb_irq errmsg;
			errmsg.type = ICE1USB_IRQ_T_ERRCNT;
			_fill_irq_err(&errmsg.u.errors, cur_err);
			printf("E");
			usb_data_write(ep_regs->bd[0].ptr, &errmsg, sizeof(errmsg));
			ep_regs->bd[0].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(sizeof(errmsg));
			usb_e1->last_err = *cur_err;
		}
	}

	/* Data IN endpoint */
	ep_regs = _get_ep_regs(USB_EP_E1_IN(port));
	bdi = usb_e1->in_bdi;

	while ((ep_regs->bd[bdi].csr & USB_BD_STATE_MSK) != USB_BD_STATE_RDY_DATA)
	{
		uint32_t ptr = ep_regs->bd[bdi].ptr;
		uint32_t hdr;
		unsigned int pos;

		/* Error check */
		if ((ep_regs->bd[bdi].csr & USB_BD_STATE_MSK) == USB_BD_STATE_DONE_ERR)
			puts("Err EP IN\n");

		/* Get some data from E1 */
		int n = e1_rx_level(port);

		if (n > 32)
			n = 9;
		else if (n > 8)
			n = 8;
		else if (!n)
			break;

		n = e1_rx_need_data(port, (ptr >> 2) + 1, n, &pos);

		/* Write header: currently version and pos (mfr/fr number) */
		hdr = (0 << 28) | (pos & 0xff);
		usb_data_write(ptr, &hdr, 4);

		/* Resubmit */
		ep_regs->bd[bdi].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN((n * 32) + 4);

		/* Next BDI */
		bdi ^= 1;
		usb_e1->in_bdi = bdi;
	}

	/* Data OUT endpoint */
	ep_regs = _get_ep_regs(USB_EP_E1_OUT(port));
	bdi = usb_e1->out_bdi;

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
			e1_tx_feed_data(port, (ptr >> 2) + 1, n);

refill:
		/* Refill it */
		ep_regs->bd[bdi].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(292);

		/* Next BDI */
		bdi ^= 1;
		usb_e1->out_bdi = bdi;
	}

	/* Feedback endpoint */
	_usb_fill_feedback_ep(port);
}

void
usb_e1_poll(void)
{
	for (int i=0; i<2; i++) {
		e1_poll(i);
		_usb_e1_run(i);
	}
}


static enum usb_fnd_resp
_e1_set_conf(const struct usb_conf_desc *conf)
{
	const struct usb_intf_desc *intf;

	printf("e1 set_conf %08x\n", conf);
	if (!conf)
		return USB_FND_SUCCESS;

	for (int port=0; port<2; port++)
	{
		intf = usb_desc_find_intf(conf, USB_INTF_E1(port), 0, NULL);
		if (!intf)
			return USB_FND_ERROR;

		printf("e1 set_conf[%d] %08x\n", port, intf);

		usb_ep_boot(intf, USB_EP_E1_IN(port),  true);
		usb_ep_boot(intf, USB_EP_E1_OUT(port), true);
		usb_ep_boot(intf, USB_EP_E1_FB(port),  false);
		usb_ep_boot(intf, USB_EP_E1_INT(port), false);
	}

	return USB_FND_SUCCESS;
}

static uint32_t
_tx_config_reg(const struct ice1usb_tx_config *cfg)
{
	return	((cfg->mode & 3) << 1) |
		((cfg->timing & 1) << 3) |
		((cfg->alarm & 1) << 4) |
		((cfg->ext_loopback & 3) << 5);
}

static uint32_t
_rx_config_reg(const struct ice1usb_rx_config *cfg)
{
	return	(cfg->mode << 1);
}

static enum usb_fnd_resp
_e1_set_intf(const struct usb_intf_desc *base, const struct usb_intf_desc *sel)
{
	volatile struct usb_ep *ep_regs;
	struct usb_e1_state *usb_e1;
	int port;

	/* Is it for E1 interface ? */
	if ((base->bInterfaceClass != 0xff) || (base->bInterfaceSubClass != 0xe1))
		return USB_FND_CONTINUE;

	/* Get matching port (if any) */
	port = _ifnum2port(base->bInterfaceNumber);
	if (port < 0)
		return USB_FND_ERROR;

	usb_e1 = _get_state(port);

	/* Valid setting ? */
	if (sel->bAlternateSetting > 1)
		return USB_FND_ERROR;

	/* Don't do anything if no change */
	if (usb_e1->running == (sel->bAlternateSetting != 0))
		return USB_FND_SUCCESS;

	usb_e1->running = (sel->bAlternateSetting != 0);

	/* Reconfigure the endpoints */
	usb_ep_reconf(sel, USB_EP_E1_IN(port));
	usb_ep_reconf(sel, USB_EP_E1_OUT(port));
	usb_ep_reconf(sel, USB_EP_E1_FB(port));
	usb_ep_reconf(sel, USB_EP_E1_INT(port));

	/* Update E1 and USB state */
	switch (usb_e1->running) {
	case false:
		/* Disable E1 rx/tx */
		e1_stop(port);
		break;

	case true:
		/* Reset and Re-Enable E1 */
		e1_start(port);

		/* Reset BDI */
		usb_e1->in_bdi = 0;
		usb_e1->out_bdi = 0;

		/* EP OUT: Queue two buffers */
		ep_regs = _get_ep_regs(USB_EP_E1_OUT(port));
		ep_regs->bd[0].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(292);
		ep_regs->bd[1].csr = USB_BD_STATE_RDY_DATA | USB_BD_LEN(292);

		break;
	}

	return USB_FND_SUCCESS;
}

static enum usb_fnd_resp
_e1_get_intf(const struct usb_intf_desc *base, uint8_t *alt)
{
	struct usb_e1_state *usb_e1;
	int port;

	/* Is it for E1 interface ? */
	if ((base->bInterfaceClass != 0xff) || (base->bInterfaceSubClass != 0xe1))
		return USB_FND_CONTINUE;

	/* Get matching port (if any) */
	port = _ifnum2port(base->bInterfaceNumber);
	if (port < 0)
		return USB_FND_ERROR;

	usb_e1 = _get_state(port);

	/* Return current alt-setting */
	*alt = usb_e1->running ? 1 : 0;

	return USB_FND_SUCCESS;
}

static bool
_set_tx_mode_done(struct usb_xfer *xfer)
{
	const struct ice1usb_tx_config *cfg = (const struct ice1usb_tx_config *) xfer->data;
	struct usb_ctrl_req *req = xfer->cb_ctx;
	int port = _ifnum2port(req->wIndex);
	struct usb_e1_state *usb_e1 = _get_state(port);
	printf("set_tx_mode[%d] %02x%02x%02x%02x\r\n", port,
		xfer->data[0], xfer->data[1], xfer->data[2], xfer->data[3]);
	usb_e1->tx_cfg = *cfg;
	e1_tx_config(port, _tx_config_reg(cfg));
	return true;
}

static bool
_set_rx_mode_done(struct usb_xfer *xfer)
{
	const struct ice1usb_rx_config *cfg = (const struct ice1usb_rx_config *) xfer->data;
	struct usb_ctrl_req *req = xfer->cb_ctx;
	int port = _ifnum2port(req->wIndex);
	struct usb_e1_state *usb_e1 = _get_state(port);
	printf("set_rx_mode[%d] %02x\r\n", port, xfer->data[0]);
	usb_e1->rx_cfg = *cfg;
	e1_rx_config(port, _rx_config_reg(cfg));
	return true;
}

static enum usb_fnd_resp
_e1_ctrl_req(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	const struct e1_error_count *cur_err;
	struct usb_e1_state *usb_e1;
	int port;

	/* Check it's for an E1 interface */
	if (USB_REQ_TYPE_RCPT(req) != (USB_REQ_TYPE_VENDOR | USB_REQ_RCPT_INTF))
		return USB_FND_CONTINUE;

	/* Get matching port (if any) */
	port = _ifnum2port(req->wIndex);
	if (port < 0)
		return USB_FND_CONTINUE;

	usb_e1 = _get_state(port);

	/* Process request */
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
		memcpy(xfer->data, &usb_e1->tx_cfg, sizeof(struct ice1usb_tx_config));
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
		memcpy(xfer->data, &usb_e1->rx_cfg, sizeof(struct ice1usb_rx_config));
		xfer->len = sizeof(struct ice1usb_rx_config);
		break;
	case ICE1USB_INTF_GET_ERRORS:
		if (req->wLength < sizeof(struct ice1usb_irq_err))
			return USB_FND_ERROR;
		cur_err = e1_get_error_count(port);
		_fill_irq_err((struct ice1usb_irq_err *)xfer->data, cur_err);
		xfer->len = sizeof(struct ice1usb_irq_err);
		break;
	default:
		return USB_FND_ERROR;
	}

	return USB_FND_SUCCESS;
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
	uint32_t rx_cr = _rx_config_reg(&rx_cfg_default);
	uint32_t tx_cr = _tx_config_reg(&tx_cfg_default);

	for (int i=0; i<2; i++) {
		struct usb_e1_state *usb_e1 = _get_state(i);
		memset(usb_e1, 0x00, sizeof(struct usb_e1_state));
		usb_e1->tx_cfg = tx_cfg_default;
		usb_e1->rx_cfg = rx_cfg_default;
		e1_init(i, rx_cr, tx_cr);
	}

	usb_register_function_driver(&_e1_drv);
}
