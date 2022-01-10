/*
 * e1_hw.h
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdint.h>

//TODO: Should latre go into the no2e1 git repo

struct e1_chan {
	uint32_t csr;
	uint32_t bd;
} __attribute__((packed,aligned(4)));

struct e1_core {
	struct e1_chan rx;
	struct e1_chan tx;
} __attribute__((packed,aligned(4)));

/* E1 receiver control register */
#define E1_RX_CR_ENABLE		(1 <<  0)	/* Enable receiver */
#define E1_RX_CR_MODE_TRSP	(0 <<  1)	/* Request no alignment at all */
#define E1_RX_CR_MODE_BYTE	(1 <<  1)	/* Request byte-level alignment */
#define E1_RX_CR_MODE_BFA	(2 <<  1)	/* Request Basic Frame Alignment */
#define E1_RX_CR_MODE_MFA	(3 <<  1)	/* Request Multi-Frame Alignment */
#define E1_RX_CR_MODE_MASK	(3 <<  1)
#define E1_RX_CR_OVFL_CLR	(1 << 12)	/* Clear Rx overflow condition */

/* E1 receiver status register */
#define E1_RX_SR_ENABLED	(1 <<  0)	/* Indicate Rx is enabled */
#define E1_RX_SR_ALIGNED	(1 <<  1)	/* Indicate Alignment achieved */
#define E1_RX_SR_BD_IN_EMPTY	(1 <<  8)
#define E1_RX_SR_BD_IN_FULL	(1 <<  9)
#define E1_RX_SR_BD_OUT_EMPTY	(1 << 10)
#define E1_RX_SR_BD_OUT_FULL	(1 << 11)
#define E1_RX_SR_OVFL		(1 << 12)	/* Indicate Rx overflow */

/* E1 transmitter control register */
#define E1_TX_CR_ENABLE		(1 <<  0)	/* Enable transmitter */
#define E1_TX_CR_MODE_TRSP	(0 <<  1)	/* Transparent bit-stream mode */
#define E1_TX_CR_MODE_TS0	(1 <<  1)	/* Generate TS0 in framer */
#define E1_TX_CR_MODE_TS0_CRC	(2 <<  1)	/* Generate TS0 + CRC4 in framer */
#define E1_TX_CR_MODE_TS0_CRC_E	(3 <<  1)	/* Generate TS0 + CRC4 + E-bits (based on Rx) in framer */
#define E1_TX_CR_MODE_MASK	(3 <<  1)
#define E1_TX_CR_TICK_LOCAL	(0 <<  3)	/* use local clock for Tx */
#define E1_TX_CR_TICK_REMOTE	(1 <<  3)	/* use recovered remote clock for Tx */
#define E1_TX_CR_TICK_MASK	(1 <<  3)
#define E1_TX_CR_ALARM		(1 <<  4)	/* indicate ALARM to remote */
#define E1_TX_CR_LOOPBACK	(1 <<  5)	/* external loopback enable/diasble */
#define E1_TX_CR_LOOPBACK_CROSS	(1 <<  6)	/* source of loopback: local (0) or other (1) port */
#define E1_TX_CR_UNFL_CLR	(1 << 12)	/* Clear Tx underflow condition */

/* E1 transmitter status register */
#define E1_TX_SR_ENABLED	(1 <<  0)	/* Indicate Tx is enabled */
#define E1_TX_SR_BD_IN_EMPTY	(1 <<  8)
#define E1_TX_SR_BD_IN_FULL	(1 <<  9)
#define E1_TX_SR_BD_OUT_EMPTY	(1 << 10)
#define E1_TX_SR_BD_OUT_FULL	(1 << 11)
#define E1_TX_SR_UNFL		(1 << 12)	/* Indicate Tx underflow */

/* E1 buffer descriptor flags */
#define E1_BD_VALID		(1 << 15)
#define E1_BD_CRC1		(1 << 14)
#define E1_BD_CRC0		(1 << 13)
#define E1_BD_ADDR(x)		((x) & 0x7f)
#define E1_BD_ADDR_MSK		0x7f
#define E1_BD_ADDR_SHFT		0
