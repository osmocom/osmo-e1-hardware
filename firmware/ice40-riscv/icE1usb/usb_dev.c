/*
 * usb_dev.c
 *
 * Copyright (C) 2019-2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include <string.h>

#include <no2usb/usb.h>
#include <no2usb/usb_proto.h>

#include "console.h"
#include "i2c.h"
#include "misc.h"

#include "ice1usb_proto.h"


const char *fw_build_str = BUILD_INFO;


static enum usb_fnd_resp
_usb_dev_ctrl_req_write(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	switch (req->bRequest) {
	case ICE1USB_DEV_I2C_REG_ACCESS:
		if (!i2c_write_reg((req->wIndex >> 8), req->wIndex & 0xff, req->wValue & 0xff))
			return USB_FND_ERROR;
		break;
	default:
		return USB_FND_ERROR;
	}

	return USB_FND_SUCCESS;
}

static enum usb_fnd_resp
_usb_dev_ctrl_req_read(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	switch (req->bRequest) {
	case ICE1USB_DEV_GET_CAPABILITIES:
		xfer->data[0] = (1 << ICE1USB_DEV_CAP_GPSDO);
		xfer->len = 1;
		break;
	case ICE1USB_DEV_GET_FW_BUILD:
		xfer->data = (void*) fw_build_str;
		xfer->len  = strlen(fw_build_str);
		break;
	case ICE1USB_DEV_I2C_REG_ACCESS:
		if (!i2c_read_reg((req->wIndex >> 8), req->wIndex & 0xff, &xfer->data[0]))
			return USB_FND_ERROR;
		xfer->len = 1;
		break;
	default:
		return USB_FND_ERROR;
	}

	return USB_FND_SUCCESS;
}

static enum usb_fnd_resp
_usb_dev_ctrl_req(struct usb_ctrl_req *req, struct usb_xfer *xfer)
{
	/* Check it's a device-wide vendor request */
	if (USB_REQ_TYPE_RCPT(req) != (USB_REQ_TYPE_VENDOR | USB_REQ_RCPT_DEV))
		return USB_FND_CONTINUE;

	/* Read / Write dispatch */
	if (USB_REQ_IS_READ(req))
		return _usb_dev_ctrl_req_read(req, xfer);
	else
		return _usb_dev_ctrl_req_write(req, xfer);
}


static struct usb_fn_drv _dev_drv = {
	.ctrl_req = _usb_dev_ctrl_req,
};

void
usb_dev_init(void)
{
	usb_register_function_driver(&_dev_drv);
}
