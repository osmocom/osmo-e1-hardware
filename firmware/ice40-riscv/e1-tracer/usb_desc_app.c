/*
 * usb_desc_app.c
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <no2usb/usb_proto.h>
#include <no2usb/usb_dfu_proto.h>
#include <no2usb/usb.h>

#define NULL ((void*)0)
#define num_elem(a) (sizeof(a) / sizeof(a[0]))


/* Legacy Configuration */
static const struct {
	struct usb_conf_desc conf;
	/* E1 */
	struct {
		struct {
			struct usb_intf_desc intf;
			struct usb_ep_desc ep_data_in0;
			struct usb_ep_desc ep_data_in1;
		} __attribute__ ((packed)) off;
		struct {
			struct usb_intf_desc intf;
			struct usb_ep_desc ep_data_in0;
			struct usb_ep_desc ep_data_in1;
		} __attribute__ ((packed)) on;
	} __attribute__ ((packed)) e1;
	/* DFU Runtime */
	struct {
		struct usb_intf_desc intf;
		struct usb_dfu_func_desc func;
	} __attribute__ ((packed)) dfu;
} __attribute__ ((packed)) _app_conf_desc = {
	.conf = {
		.bLength                = sizeof(struct usb_conf_desc),
		.bDescriptorType        = USB_DT_CONF,
		.wTotalLength           = sizeof(_app_conf_desc),
		.bNumInterfaces         = 2,
		.bConfigurationValue    = 1,
		.iConfiguration         = 4,
		.bmAttributes           = 0x80,
		.bMaxPower              = 0x32, /* 100 mA */
	},
	.e1 = {
		.off = {
			.intf = {
				.bLength		= sizeof(struct usb_intf_desc),
				.bDescriptorType	= USB_DT_INTF,
				.bInterfaceNumber	= 0,
				.bAlternateSetting	= 0,
				.bNumEndpoints		= 2,
				.bInterfaceClass	= 0xff,
				.bInterfaceSubClass	= 0xe1,
				.bInterfaceProtocol	= 0x00,
				.iInterface		= 5,
			},
			.ep_data_in0 = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= 0x81,
				.bmAttributes		= 0x05,
				.wMaxPacketSize		= 0,
				.bInterval		= 1,
			},
			.ep_data_in1 = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= 0x82,
				.bmAttributes		= 0x05,
				.wMaxPacketSize		= 0,
				.bInterval		= 1,
			},
		},
		.on = {
			.intf = {
				.bLength		= sizeof(struct usb_intf_desc),
				.bDescriptorType	= USB_DT_INTF,
				.bInterfaceNumber	= 0,
				.bAlternateSetting	= 1,
				.bNumEndpoints		= 2,
				.bInterfaceClass	= 0xff,
				.bInterfaceSubClass	= 0xe1,
				.bInterfaceProtocol	= 0x00,
				.iInterface		= 5,
			},
			.ep_data_in0 = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= 0x81,
				.bmAttributes		= 0x05,
				.wMaxPacketSize		= 388,
				.bInterval		= 1,
			},
			.ep_data_in1 = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= 0x82,
				.bmAttributes		= 0x05,
				.wMaxPacketSize		= 388,
				.bInterval		= 1,
			},
		},
	},
	.dfu = {
		.intf = {
			.bLength		= sizeof(struct usb_intf_desc),
			.bDescriptorType	= USB_DT_INTF,
			.bInterfaceNumber	= 1,
			.bAlternateSetting	= 0,
			.bNumEndpoints		= 0,
			.bInterfaceClass	= 0xfe,
			.bInterfaceSubClass	= 0x01,
			.bInterfaceProtocol	= 0x01,
			.iInterface		= 6,
		},
		.func = {
			.bLength		= sizeof(struct usb_dfu_func_desc),
			.bDescriptorType	= USB_DFU_DT_FUNC,
			.bmAttributes		= 0x0d,
			.wDetachTimeOut		= 1000,
			.wTransferSize		= 4096,
			.bcdDFUVersion		= 0x0101,
		},
	},
};

/* "icE1usb" compatible Configuration */
static const struct {
	struct usb_conf_desc conf;
	/* Interface / Direction A */
	struct {
		struct {
			struct usb_intf_desc intf;
		} __attribute__ ((packed)) off;
		struct {
			struct usb_intf_desc intf;
			struct usb_ep_desc ep_data_in0;
		} __attribute__ ((packed)) on;
	} __attribute__ ((packed)) a;
	/* Interface / Direction B */
	struct {
		struct {
			struct usb_intf_desc intf;
		} __attribute__ ((packed)) off;
		struct {
			struct usb_intf_desc intf;
			struct usb_ep_desc ep_data_in1;
		} __attribute__ ((packed)) on;
	} __attribute__ ((packed)) b;
} __attribute__ ((packed)) _app_conf_desc_e1d = {
	.conf = {
		.bLength                = sizeof(struct usb_conf_desc),
		.bDescriptorType        = USB_DT_CONF,
		.wTotalLength           = sizeof(_app_conf_desc_e1d),
		.bNumInterfaces         = 2,
		.bConfigurationValue    = 2,
		.iConfiguration         = 7,
		.bmAttributes           = 0x80,
		.bMaxPower              = 0x32, /* 100 mA */
	},
	.a = {
		.off = {
			.intf = {
				.bLength		= sizeof(struct usb_intf_desc),
				.bDescriptorType	= USB_DT_INTF,
				.bInterfaceNumber	= 0,
				.bAlternateSetting	= 0,
				.bNumEndpoints		= 0,
				.bInterfaceClass	= 0xff,
				.bInterfaceSubClass	= 0xe1,
				.bInterfaceProtocol	= 0x00,
				.iInterface		= 8,
			},
		},
		.on = {
			.intf = {
				.bLength		= sizeof(struct usb_intf_desc),
				.bDescriptorType	= USB_DT_INTF,
				.bInterfaceNumber	= 0,
				.bAlternateSetting	= 1,
				.bNumEndpoints		= 1,
				.bInterfaceClass	= 0xff,
				.bInterfaceSubClass	= 0xe1,
				.bInterfaceProtocol	= 0x00,
				.iInterface		= 8,
			},
			.ep_data_in0 = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= 0x81,
				.bmAttributes		= 0x05,
				.wMaxPacketSize		= 388,
				.bInterval		= 1,
			},
		},
	},
	.b = {
		.off = {
			.intf = {
				.bLength		= sizeof(struct usb_intf_desc),
				.bDescriptorType	= USB_DT_INTF,
				.bInterfaceNumber	= 1,
				.bAlternateSetting	= 0,
				.bNumEndpoints		= 0,
				.bInterfaceClass	= 0xff,
				.bInterfaceSubClass	= 0xe1,
				.bInterfaceProtocol	= 0x00,
				.iInterface		= 9,
			},
		},
		.on = {
			.intf = {
				.bLength		= sizeof(struct usb_intf_desc),
				.bDescriptorType	= USB_DT_INTF,
				.bInterfaceNumber	= 1,
				.bAlternateSetting	= 1,
				.bNumEndpoints		= 1,
				.bInterfaceClass	= 0xff,
				.bInterfaceSubClass	= 0xe1,
				.bInterfaceProtocol	= 0x00,
				.iInterface		= 9,
			},
			.ep_data_in1 = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= 0x82,
				.bmAttributes		= 0x05,
				.wMaxPacketSize		= 388,
				.bInterval		= 1,
			},
		},
	},
};

static const struct usb_conf_desc * const _conf_desc_array[] = {
	&_app_conf_desc.conf,
	&_app_conf_desc_e1d.conf,
};

static const struct usb_dev_desc _dev_desc = {
	.bLength		= sizeof(struct usb_dev_desc),
	.bDescriptorType	= USB_DT_DEV,
	.bcdUSB			= 0x0200,
	.bDeviceClass		= 0,
	.bDeviceSubClass	= 0,
	.bDeviceProtocol	= 0,
	.bMaxPacketSize0	= 64,
	.idVendor		= 0x1d50,
	.idProduct		= 0x6151,
	.bcdDevice		= 2,
	.iManufacturer		= 2,
	.iProduct		= 3,
	.iSerialNumber		= 1,
	.bNumConfigurations	= num_elem(_conf_desc_array),
};

#include "usb_str_app.gen.h"

const struct usb_stack_descriptors app_stack_desc = {
	.dev = &_dev_desc,
	.conf = _conf_desc_array,
	.n_conf = num_elem(_conf_desc_array),
	.str = _str_desc_array,
	.n_str = num_elem(_str_desc_array),
};
