/*
 * usb_gpsdo.c
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

#include "usb_desc_ids.h"
#include "gpsdo.h"

#include "ice1usb_proto.h"

static void
_get_gpsdo_status(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	struct e1usb_gpsdo_status status;

	gpsdo_get_status(&status);

	memcpy(xfer->data, &status, sizeof(struct e1usb_gpsdo_status));
	xfer->len = sizeof(struct e1usb_gpsdo_status);
}

static void
_get_gpsdo_mode(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	xfer->data[0] = gpsdo_enabled() ? ICE1USB_GPSDO_MODE_DISABLED : ICE1USB_GPSDO_MODE_AUTO;
	xfer->len = 1;
}

static void
_set_gpsdo_mode(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	gpsdo_enable(req->wValue != ICE1USB_GPSDO_MODE_DISABLED);
}

static void
_get_gpsdo_tune(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	uint16_t coarse, fine;
	struct e1usb_gpsdo_tune tune;

	gpsdo_get_tune(&coarse, &fine);
	tune.coarse = coarse;
	tune.fine = fine;

	memcpy(xfer->data, &tune, sizeof(struct e1usb_gpsdo_tune));
	xfer->len = sizeof(struct e1usb_gpsdo_tune);
}

static bool
_set_gpsdo_tune_done(struct usb_xfer *xfer)
{
	const struct e1usb_gpsdo_tune *tune = (const void *) xfer->data;
	gpsdo_set_tune(tune->coarse, tune->fine);
	return true;
}

static void
_set_gpsdo_tune(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	xfer->cb_done = _set_gpsdo_tune_done;
	xfer->cb_ctx  = req;
	xfer->len     = sizeof(struct e1usb_gpsdo_tune);
}


static enum usb_fnd_resp
_gpsdo_ctrl_req(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	/* Check it's for an interface */
	if (USB_REQ_TYPE_RCPT(req) != (USB_REQ_TYPE_VENDOR | USB_REQ_RCPT_INTF))
		return USB_FND_CONTINUE;

	/* Check it's for the GPS-DO interface */
	if (req->wIndex != USB_INTF_GPSDO)
		return USB_FND_CONTINUE;

	switch (req->bRequest) {
	case ICE1USB_INTF_GET_GPSDO_STATUS:
		_get_gpsdo_status(req, xfer);
		break;
	case ICE1USB_INTF_GET_GPSDO_MODE:
		_get_gpsdo_mode(req, xfer);
		break;
	case ICE1USB_INTF_SET_GPSDO_MODE:
		_set_gpsdo_mode(req, xfer);
		break;
	case ICE1USB_INTF_GET_GPSDO_TUNE:
		_get_gpsdo_tune(req, xfer);
		break;
	case ICE1USB_INTF_SET_GPSDO_TUNE:
		_set_gpsdo_tune(req, xfer);
		break;
	default:
		return USB_FND_ERROR;
	}

	return USB_FND_SUCCESS;
}

static struct usb_fn_drv _gpsdo_drv = {
	.ctrl_req	= _gpsdo_ctrl_req,
};

void
usb_gpsdo_init(void)
{
	usb_register_function_driver(&_gpsdo_drv);
}
