/*
 * usb_desc_ids.h
 *
 * Copyright (C) 2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#define USB_INTF_E1(p)		(0 + (p))
#define USB_INTF_DFU		1
#define USB_INTF_NUM		2

#define USB_EP_E1_IN(p)		(0x82 + (3 * (p)))
#define USB_EP_E1_OUT(p)	(0x01 + (3 * (p)))
#define USB_EP_E1_FB(p)		(0x81 + (3 * (p)))
#define USB_EP_E1_INT(p)	(0x83 + (3 * (p)))
