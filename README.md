osmo-e1-hardware - Collection of various E1/TDM hardware projects
=================================================================

This repository contains a variety of different projects related to
E1/TDM hardware interfaces

* `icE1usb` (fully-fledged USB E1 interface adapter)
* `osmo-e1-tracer` (fully integrated passive raw bitstream tracer)
* `osmo-e1-tap` (passive high-impedance E1/T1 tap)
* `osmo-e1-xcvr` (E1 LIU + magnetics)
  https://osmocom.org/projects/e1-t1-adapter/wiki/Osmo-e1-xcvr

icE1usb
-------

The Osmocom icE1usb project is an open source hardware, gateware and firmware
project implementing a USB-attached interface for E1 circuits.  Use this
if you want to implement a system speaking to an E1 circuit, such as
* a GSM BSC (using [OsmoBSC](https://osmocom.org/projects/osmobsc/wiki)) talking Abis/E1 to a GSM Base station,
* using [osmo-gbproxy](https://osmocom.org/projects/osmo-gbproxy/wiki/Osmo-gbproxy) to convert classing Gb/E1 to Gb/IP
* talking to legacy ISDN PRI equipment such as PBX, RAS servers, etc.
* connecting legacy ISDN PRI equipment such as PBX, RAS servers, etc. to
  the [OCTOI comminity TDMoIP network](https://osmocom.org/projects/octoi/wiki)

Depending on your USB host controller, it supports one or two E1
circuits. Most host controllers can only support one E1 circuit.

In order to provide a stable E1 clock reference, it contains a GPS disciplined oscillator.

See <https://osmocom.org/projects/e1-t1-adapter/wiki/IcE1usb> for more details.

Fully assembled units can be purchased at the [sysmocom
webshop](https://shop.sysmocom.de/Osmocom-icE1usb-E1-interface-for-USB/icE1usb-kit)

Check the `hardware/icE1usb`, `firmware/ice40-riscv/icE1usb` directories in this repository.

The host software is provided either
* via [osmo-e1d](https://osmocom.org/projects/osmo-e1d/wiki), or
* via the [icE1usb DAHDI driver](https://gitea.osmocom.org/retronetworking/dahdi-linux)


osmo-e1-tracer
--------------

This is a fully integrated design that allows you to obtain bi-directional high-impedance
bitstream E1 traces.  It features an iCE40 FPGA with USB + E1 cores from Sylvain Munaut,
as well as two E1 LIUs.

See <https://osmocom.org/projects/e1-t1-adapter/wiki/E1_tracer> for more details.

Fully assembled units can be purchased made-to-order from
[sysmocom](https://sysmocom.de).

Check the `hardware/e1-tracer`, `firmware/ice40-riscv/e1-tracer` and `software/e1-tracer`
directories in this repository.


osmo-e1-xcvr
------------

This was  a simple hardware project that aims to generate a reusable module
for interfacing E1/T1/J1 lines from various custom FPGA/CPLD/microcontroller
projects.  Consider it part of an earlier R&D setup before icE1usb and
osmo-e1-tracer were around.

The board contains transformers, the analog circuitry, the LIU (line interface
unit), an oscillator as well as an integrated transceiver chip.

It exposes the control interface (SPI) as well as the decoded synchronous
Rx/Tx bitstreams each on a 2x5pin header.

Framer, Multiplexer, HDLC decoder or anything like that is out-of-scope for
now.  The idea really is to provide an interface as low-level as possible.

One of the ideas is to create a "soft E1" interface, where the Rx/Tx bitstreams
are interfaced with the SSC of an AT91SAM3S and subsequently passed into a PC
via USB.  The 2Mbps signal is very low-bandwidth, so that a pure software
implementation should be absolutely no problem for todays computing power.

See <https://osmocom.org/projects/e1-t1-adapter/wiki/Osmo-e1-xcvr> for more details

Check the `hardware/e1-xcvr` directory in this repository.


osmo-e1-tap
-----------

This is a small passive board that allows you to perform high-impedance tracing on an E1
or T1 line.

Fully assembled units can be purchased at the [sysmocom
webshop](https://shop.sysmocom.de/E1-T1-tap-adapter/e1-tap).

Check the `hardware/e1-tap` directory in this repository.
