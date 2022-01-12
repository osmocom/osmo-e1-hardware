/*
 * usb_dev.c
 *
 * Copyright (C) 2019-2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>

#include <no2usb/usb.h>
#include <no2usb/usb_proto.h>

#include "console.h"
#include "misc.h"

#include "ice1usb_proto.h"


static enum usb_fnd_resp
_usb_dev_ctrl_req(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	/* Check it's a device-wide vendor request */
	if (USB_REQ_TYPE_RCPT(req) != (USB_REQ_TYPE_VENDOR | USB_REQ_RCPT_DEV))
		return USB_FND_CONTINUE;

	/* Dispatch / Handle */
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


static struct usb_fn_drv _dev_drv = {
	.ctrl_req = _usb_dev_ctrl_req,
};

void
usb_dev_init(void)
{
	usb_register_function_driver(&_dev_drv);
}
