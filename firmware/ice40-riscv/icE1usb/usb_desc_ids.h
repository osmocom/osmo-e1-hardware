/*
 * usb_desc_ids.h
 *
 * Copyright (C) 2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#define USB_INTF_E1(p)		(0 + (p))
#define USB_INTF_GPS_CDC_CTL	(NUM_E1_PORTS + 0)
#define USB_INTF_GPS_CDC_DATA	(NUM_E1_PORTS + 1)
#define USB_INTF_GPSDO		(NUM_E1_PORTS + 2)
#define USB_INTF_DFU		(NUM_E1_PORTS + 3)
#define USB_INTF_NUM		(NUM_E1_PORTS + 4)

#define USB_EP_E1_IN(p)		(0x82 + (3 * (p)))
#define USB_EP_E1_OUT(p)	(0x01 + (3 * (p)))
#define USB_EP_E1_FB(p)		(0x81 + (3 * (p)))
#define USB_EP_E1_INT(p)	(0x83 + (3 * (p)))

#define USB_EP_GPS_CDC_CTL	0x88
#define USB_EP_GPS_CDC_OUT	0x09
#define USB_EP_GPS_CDC_IN	0x89
