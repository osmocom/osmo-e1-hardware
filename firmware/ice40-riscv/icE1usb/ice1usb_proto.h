#pragma once

/* Header file describing the USB protocol between the icE1usb firmware and the host
 * software (currently really only osmo-e1d) */

/* Device Requests */

/*! returns a bit-mask of optional device capabilities (see enum e1usb_dev_capability) */
#define ICE1USB_DEV_GET_CAPABILITIES	0x01
/*! returns a 
#define ICE1USB_DEV_GET_FW_BUILD	0x02

enum e1usb_dev_capability {
	/*! Does this board have a GPS-DO */
	ICE1USB_DEV_CAP_GPSDO,
};


/* Interface Requests */

/*! returns a bit-mask of optional device capabilities (see enum e1usb_intf_capability) */
#define ICE1USB_INTF_GET_CAPABILITIES	0x01
#define ICE1USB_INTF_SET_TX_CFG		0x02	/*!< struct ice1usb_tx_config */
#define ICE1USB_INTF_GET_TX_CFG		0x03	/*!< struct ice1usb_tx_config */
#define ICE1USB_INTF_SET_RX_CFG		0x04	/*!< struct ice1usb_rx_config */
#define ICE1USB_INTF_GET_RX_CFG		0x05	/*!< struct ice1usb_rx_config */

//enum e1usb_intf_capability { };

enum ice1usb_tx_mode {
	ICE1USB_TX_MODE_TRANSP		= 0,
	ICE1USB_TX_MODE_TS0		= 1,
	ICE1USB_TX_MODE_TS0_CRC4	= 2,
	ICE1USB_TX_MODE_TS0_CRC4_E	= 3,
};

enum ice1usb_tx_timing {
	ICE1USB_TX_TIME_SRC_REMOTE	= 0,
	ICE1USB_TX_TIME_SRC_LOCAL	= 1,
};

enum ice1usb_tx_ext_loopback {
	ICE1USB_TX_EXT_LOOPBACK_OFF	= 0,
	ICE1USB_TX_EXT_LOOPBACK_SAME	= 1,
	ICE1USB_TX_EXT_LOOPBACK_CROSS	= 2,
};

/* ICE1USB_INTF_{GET,SET}_TX_CFG */
struct ice1usb_tx_config {
	uint8_t mode;		/*!< enum ice1usb_tx_mode */
	uint8_t timing;		/*!< enum ice1usb_tx_timing */
	uint8_t ext_loopback;	/*!< enum ice1usb_tx_ext_loopback */
	uint8_t alarm;		/*!< 1 = transmit alarm; 0 = don't */
} __attribute__((packed));


enum ice1usb_rx_mode {
	/*! transparent, unaligned bitstream */
	ICE1USB_RX_MODE_TRANSP		= 0,
	/*! alignment to E1 frame */
	ICE1USB_RX_MODE_FRAME		= 1,
	/*! alignment to E1 multiframe */
	ICE1USB_RX_MODE_MULTIFRAME	= 2,
};

/* ICE1USB_INTF_{GET,SET}_RX_CFG */
struct ice1usb_rx_config {
	uint8_t mode;		/*!< enum ice1usb_rx_mode */
} __attribute__((packed));
