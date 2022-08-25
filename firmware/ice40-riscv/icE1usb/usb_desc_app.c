/*
 * usb_desc_app.c
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <no2usb/usb_proto.h>
#include <no2usb/usb_cdc_proto.h>
#include <no2usb/usb_dfu_proto.h>
#include <no2usb/usb.h>

#include "usb_desc_ids.h"

#define NULL ((void*)0)
#define num_elem(a) (sizeof(a) / sizeof(a[0]))


usb_cdc_union_desc_def(1);

static const struct {
	/* Configuration */
	struct usb_conf_desc conf;

	/* E1 */
	struct {
		/* Two altsettings are required, as isochronous
		 * interfaces must have a setting where they don't
		 * transceive any data. We just remove the isochronous
		 * endpoints in the 'off' altsetting */
		struct {
			struct usb_intf_desc intf;
			struct usb_ep_desc ep_interrupt;
		} __attribute__ ((packed)) off;
		struct {
			struct usb_intf_desc intf;
			struct usb_ep_desc ep_data_in;
			struct usb_ep_desc ep_data_out;
			struct usb_ep_desc ep_fb;
			struct usb_ep_desc ep_interrupt;
		} __attribute__ ((packed)) on;
	} __attribute__ ((packed)) e1[2];

	/* CDC */
	struct {
		struct usb_intf_desc intf_ctl;
		struct usb_cdc_hdr_desc cdc_hdr;
		struct usb_cdc_acm_desc cdc_acm;
		struct usb_cdc_union_desc__1 cdc_union;
		struct usb_ep_desc ep_ctl;
		struct usb_intf_desc intf_data;
		struct usb_ep_desc ep_data_out;
		struct usb_ep_desc ep_data_in;
	} __attribute__ ((packed)) cdc;

	/* GPS-DO (control EP only) */
	struct {
		struct usb_intf_desc intf;
	} __attribute__ ((packed)) gpsdo;

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
		.bNumInterfaces         = USB_INTF_NUM,
		.bConfigurationValue    = 1,
		.iConfiguration         = 4,
		.bmAttributes           = 0x80,
		.bMaxPower              = 0x32, /* 100 mA */
	},
	.e1[0] = {
		.off = {
			.intf = {
				.bLength		= sizeof(struct usb_intf_desc),
				.bDescriptorType	= USB_DT_INTF,
				.bInterfaceNumber	= USB_INTF_E1(0),
				.bAlternateSetting	= 0,
				.bNumEndpoints		= 1,
				.bInterfaceClass	= 0xff,
				.bInterfaceSubClass	= 0xe1,
				.bInterfaceProtocol	= 0x00,
				.iInterface		= 5,
			},
			.ep_interrupt = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= USB_EP_E1_INT(0),
				.bmAttributes		= 0x03,
				.wMaxPacketSize		= 10,
				.bInterval		= 4,	/* every 4 ms */
			},
		},
		.on = {
			.intf = {
				.bLength		= sizeof(struct usb_intf_desc),
				.bDescriptorType	= USB_DT_INTF,
				.bInterfaceNumber	= USB_INTF_E1(0),
				.bAlternateSetting	= 1,
				.bNumEndpoints		= 4,
				.bInterfaceClass	= 0xff,
				.bInterfaceSubClass	= 0xe1,
				.bInterfaceProtocol	= 0x00,
				.iInterface		= 6,
			},
			.ep_data_in = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= USB_EP_E1_IN(0),
				.bmAttributes		= 0x05,
				.wMaxPacketSize		= 292,
				.bInterval		= 1,
			},
			.ep_data_out = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= USB_EP_E1_OUT(0),
				.bmAttributes		= 0x05,
				.wMaxPacketSize		= 292,
				.bInterval		= 1,
			},
			.ep_fb = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= USB_EP_E1_FB(0),
				.bmAttributes		= 0x11,
				.wMaxPacketSize		= 3,
				.bInterval		= 3,	/* every 2^(3-1) = 4 ms */
			},
			.ep_interrupt = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= USB_EP_E1_INT(0),
				.bmAttributes		= 0x03,
				.wMaxPacketSize		= 10,
				.bInterval		= 4,	/* every 4 ms */
			},
		},
	},
	.e1[1] = {
		.off = {
			.intf = {
				.bLength		= sizeof(struct usb_intf_desc),
				.bDescriptorType	= USB_DT_INTF,
				.bInterfaceNumber	= USB_INTF_E1(1),
				.bAlternateSetting	= 0,
				.bNumEndpoints		= 1,
				.bInterfaceClass	= 0xff,
				.bInterfaceSubClass	= 0xe1,
				.bInterfaceProtocol	= 0x00,
				.iInterface		= 7,
			},
			.ep_interrupt = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= USB_EP_E1_INT(1),
				.bmAttributes		= 0x03,
				.wMaxPacketSize		= 10,
				.bInterval		= 4,	/* every 4 ms */
			},
		},
		.on = {
			.intf = {
				.bLength		= sizeof(struct usb_intf_desc),
				.bDescriptorType	= USB_DT_INTF,
				.bInterfaceNumber	= USB_INTF_E1(1),
				.bAlternateSetting	= 1,
				.bNumEndpoints		= 4,
				.bInterfaceClass	= 0xff,
				.bInterfaceSubClass	= 0xe1,
				.bInterfaceProtocol	= 0x00,
				.iInterface		= 8,
			},
			.ep_data_in = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= USB_EP_E1_IN(1),
				.bmAttributes		= 0x05,
				.wMaxPacketSize		= 292,
				.bInterval		= 1,
			},
			.ep_data_out = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= USB_EP_E1_OUT(1),
				.bmAttributes		= 0x05,
				.wMaxPacketSize		= 292,
				.bInterval		= 1,
			},
			.ep_fb = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= USB_EP_E1_FB(1),
				.bmAttributes		= 0x11,
				.wMaxPacketSize		= 3,
				.bInterval		= 3,	/* every 2^(3-1) = 4 ms */
			},
			.ep_interrupt = {
				.bLength		= sizeof(struct usb_ep_desc),
				.bDescriptorType	= USB_DT_EP,
				.bEndpointAddress	= USB_EP_E1_INT(1),
				.bmAttributes		= 0x03,
				.wMaxPacketSize		= 10,
				.bInterval		= 4,	/* every 4 ms */
			},
		},
	},
	.cdc = {
		.intf_ctl = {
			.bLength		= sizeof(struct usb_intf_desc),
			.bDescriptorType	= USB_DT_INTF,
			.bInterfaceNumber	= USB_INTF_GPS_CDC_CTL,
			.bAlternateSetting	= 0,
			.bNumEndpoints		= 1,
			.bInterfaceClass	= USB_CLS_CDC_CONTROL,
			.bInterfaceSubClass	= USB_CDC_SCLS_ACM,
			.bInterfaceProtocol	= 0x00,
			.iInterface		= 9,
		},
		.cdc_hdr = {
			.bLength		= sizeof(struct usb_cdc_hdr_desc),
			.bDescriptorType	= USB_CS_DT_INTF,
			.bDescriptorsubtype	= USB_CDC_DST_HEADER,
			.bcdCDC			= 0x0110,
		},
		.cdc_acm = {
			.bLength		= sizeof(struct usb_cdc_acm_desc),
			.bDescriptorType	= USB_CS_DT_INTF,
			.bDescriptorsubtype	= USB_CDC_DST_ACM,
				/* Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding, */
				/* and the notification Serial_State */
			.bmCapabilities		= 0x02,
		},
		.cdc_union = {
			.bLength		= sizeof(struct usb_cdc_union_desc) + 1,
			.bDescriptorType	= USB_CS_DT_INTF,
			.bDescriptorsubtype	= USB_CDC_DST_UNION,
			.bMasterInterface	= USB_INTF_GPS_CDC_CTL,
			.bSlaveInterface	= { USB_INTF_GPS_CDC_DATA },
		},
		.ep_ctl = {
			.bLength		= sizeof(struct usb_ep_desc),
			.bDescriptorType	= USB_DT_EP,
			.bEndpointAddress	= USB_EP_GPS_CDC_CTL,
			.bmAttributes		= 0x03,
				/* Longest notif is SERIAL_STATE with 2 data bytes */
			.wMaxPacketSize		= sizeof(struct usb_ctrl_req) + 2,
#ifdef GPS_PPS_ON_CD
			.bInterval		= 1,
#else
			.bInterval		= 0x40,
#endif
		},
		.intf_data = {
			.bLength		= sizeof(struct usb_intf_desc),
			.bDescriptorType	= USB_DT_INTF,
			.bInterfaceNumber	= USB_INTF_GPS_CDC_DATA,
			.bAlternateSetting	= 0,
			.bNumEndpoints		= 2,
			.bInterfaceClass	= USB_CLS_CDC_DATA,
			.bInterfaceSubClass	= 0x00,
			.bInterfaceProtocol	= 0x00,
			.iInterface		= 10,
		},
		.ep_data_out = {
			.bLength		= sizeof(struct usb_ep_desc),
			.bDescriptorType	= USB_DT_EP,
			.bEndpointAddress	= USB_EP_GPS_CDC_OUT,
			.bmAttributes		= 0x02,
			.wMaxPacketSize		= 64,
			.bInterval		= 0x00,
		},
		.ep_data_in = {
			.bLength		= sizeof(struct usb_ep_desc),
			.bDescriptorType	= USB_DT_EP,
			.bEndpointAddress	= USB_EP_GPS_CDC_IN,
			.bmAttributes		= 0x02,
			.wMaxPacketSize		= 64,
			.bInterval		= 0x00,
		},
	},
	.gpsdo = {
		.intf = {
			.bLength		= sizeof(struct usb_intf_desc),
			.bDescriptorType	= USB_DT_INTF,
			.bInterfaceNumber	= USB_INTF_GPSDO,
			.bAlternateSetting	= 0,
			.bNumEndpoints		= 0,
			.bInterfaceClass	= 0xff,
			.bInterfaceSubClass	= 0xe1,
			.bInterfaceProtocol	= 0xd0,
			.iInterface		= 11,
		}
	},
	.dfu = {
		.intf = {
			.bLength		= sizeof(struct usb_intf_desc),
			.bDescriptorType	= USB_DT_INTF,
			.bInterfaceNumber	= USB_INTF_DFU,
			.bAlternateSetting	= 0,
			.bNumEndpoints		= 0,
			.bInterfaceClass	= 0xfe,
			.bInterfaceSubClass	= 0x01,
			.bInterfaceProtocol	= 0x01,
			.iInterface		= 12,
		},
		.func = {
			.bLength		= sizeof(struct usb_dfu_func_desc),
			.bDescriptorType	= USB_DFU_DT_FUNC,
			.bmAttributes		= 0x0d,
			.wDetachTimeOut		= 0,
			.wTransferSize		= 4096,
			.bcdDFUVersion		= 0x0101,
		},
	},
};

static const struct usb_conf_desc * const _conf_desc_array[] = {
	&_app_conf_desc.conf,
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
	.idProduct		= 0x6145,
	.bcdDevice		= 0x0003,	/* v0.3 */
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
