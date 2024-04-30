icE1usb-proto SoC Memory Map
============================

Overview
--------

| Base        | Size             | Description     | IP Core doc
|-------------|------------------|-----------------|-------------
|`0x00000000` | `0x00400` (1k)   | Boot ROM        |
|`0x00020000` | `0x10000` (64k)  | Main SRAM       |
|`0x80000000` |                  | Flash SPI       | [`no2ice40/ice40_spi_wb`](../../cores/no2ice40/doc/ice40_spi_wb.md)
|`0x81000000` |                  | Debug UART      | [`no2misc/uart_wb`](../../cores/no2misc/doc/uart_wb.md)
|`0x82000000` |                  | RGB LED         | [`no2ice40/ice40_rgb_wb`](../../cores/no2ice40/doc/ice40_rgb_wb.md)
|`0x83000000` |                  | USB core        | [`no2usb`](../../cores/no2usb/doc/mem-map.md)
|`0x84000000` | `0x01000` (4k)   | USB data buffer |
|`0x85000000` | `0x10000` (64k)  | E1 data buffer  |
|`0x86000000` |                  | DMA             | See below
|`0x87000000` |                  | E1 core         | [`no2e1`](../../cores/no2e1/doc/mem-map.md). See notes below.
|`0x88000000` |                  | Misc            | See below


Memory
------

### `0x00000000`: Boot ROM

This memory zone is initialized directly in the FPGA bitstream itself.
It contains the bootstrap program that will load the main firmware from
flash into the main SRAM and jump to it.


### `0x00020000`: Main SRAM

The main SoC SRAM, implemented using the UP5k SPRAMs. It can't be initialized
in the FPGA bitstream directly, hence the need for the Boot ROM zone.


### `0x84000000`: USB data buffer

This 4k zone actually correspond to 8k of SRAM inside the USB core.
The RX and TX buffer for the USB are distinct and are accessed independently
depending if read or write access are made to this zone. (RX buffer can only
be read, TX buffer can only be written). All accesses must also be 32 bit
wide !


### `0x85000000`: E1 data buffer

This 64k buffer is reserved for E1 data and is what is used by the E1 core.

Address mapping is :

```text
,---------------------------------------------------------------,
| f | e | d | c | b | a | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|---------------------------------------------------------------|
|        multi-frame        |     frame     |    timeslot       |
'---------------------------------------------------------------'
```

and the `multi-frame` number is what is exchanged in the E1 core buffer
descriptors.


Custom Peripherals
------------------

### `0x86000000`: DMA

Very simple DMA core allowing direct copy of data between the USB and E1 data
buffers.

#### Status (Read Only, addr `0x00`)

```text
,-----------------------------------------------------------------------------------------------,
|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
|-----------------------------------------------------------------------------------------------|
|                             /                 | b| d| /|               len                    |
'-----------------------------------------------------------------------------------------------'

 * [   15] - b   : Busy flag
 * [   14] - d   : Direction ( 0=E1 to USB, 1=USB to E1 )
 * [12: 0] - len : Remaining length of the in-progress transfer ( -1 = done )
```

#### Transfer start/direction/length (Write Only, addr `0x00`)

Write to this register initiate the transfer.

```text
,-----------------------------------------------------------------------------------------------,
|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
|-----------------------------------------------------------------------------------------------|
|                             /                    | d|  /  |             len                   |
'-----------------------------------------------------------------------------------------------'

 * [   14] - d   : Direction ( 0=E1 to USB, 1=USB to E1 )
 * [11: 0] - len : Transfer length ( Number of Words - 2 )
```

#### E1 buffer offset (Write Only, addr `0x08`)

```text
,-----------------------------------------------------------------------------------------------,
|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
|-----------------------------------------------------------------------------------------------|
|                     /                               |           e1_ofs                        |
'-----------------------------------------------------------------------------------------------'

 * [13:0] - e1_ofs
```

Word offset inside the E1 data buffer where the next transfer will start

#### USB buffer offset (Write Only, addr `0x0C`)

```text
,-----------------------------------------------------------------------------------------------,
|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
|-----------------------------------------------------------------------------------------------|
|                              /                                  |        usb_ofs              |
'-----------------------------------------------------------------------------------------------'

 * [9:0] - usb_ofs
```

Word offset inside the USB End Point buffer where the next transfer will start


### `0x87000000`: E1 core

Refer to the [`no2e1` core documentation](../../cores/no2e1/doc/mem-map.md)
for register description.

The core instanciated here has a 1 single channel containing both
RX and TX units.


### `0x88000000`: Misc

Collection of small auxiliary peripherals.

#### Boot (Write Only, addr `0x00`)

```text
,-----------------------------------------------------------------------------------------------,
|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
|-----------------------------------------------------------------------------------------------|
|                                         /                                            | x| sel |
'-----------------------------------------------------------------------------------------------'

 * [   2] - d   : boot eXecute
 * [11:0] - sel : boot select
```

Write to this register with bit 2 set will trigger a FPGA reload of the selected image.

#### E1 tick source select (Write Only, addr `0x04`)

```text
,-----------------------------------------------------------------------------------------------,
|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
|-----------------------------------------------------------------------------------------------|
|                                        /                                                | tss |
'-----------------------------------------------------------------------------------------------'

 * [ 1: 0] - tss : Tick Source Select
```

Selects the tick source for the counters below.
Available sources are :

* `0`: TX tick, for every bit sent
* `1`: RX Pulse, for every detected pulse on the input
* `2`: RX Sample, for every bit received
* `3`: RX One, for every `1` bit received

#### E1 tick counters (Read Only, addr `0x04`)

```text
,-----------------------------------------------------------------------------------------------,
|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
|-----------------------------------------------------------------------------------------------|
|                     /                         |                   tick_cnt                    |
'-----------------------------------------------------------------------------------------------'

 * [15: 0] - tick_cnt : Tick counter value
```

An internal counter is incremented at every E1 tick (tick source selected above) for each E1
channel. That counter value is then captured at every USB Start-of-Frame packet and the last
captured value made available here.

#### Time (Read Only, addr `0x07`)

```text
,-----------------------------------------------------------------------------------------------,
|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
|-----------------------------------------------------------------------------------------------|
|                                             time                                              |
'-----------------------------------------------------------------------------------------------'

 * [31:0] - time
```

32 bit counter incremented at the system clock rate ( 30.72 MHz )


#### PDM (Write Only, addr `0x08-0x0f`)

This exposes configurable voltages ( DACs ). Some channels are 12 bits, some are 8 bits.

```text
12 bits:

,-----------------------------------------------------------------------------------------------,
|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
|-----------------------------------------------------------------------------------------------|
|en|                        /                               |            value                  |
'-----------------------------------------------------------------------------------------------'

 * [31  ] - enable (output tristated if 0)
 * [11:0] - value


 8 bits:

,-----------------------------------------------------------------------------------------------,
|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
|-----------------------------------------------------------------------------------------------|
|en|                               /                                    |      value            |
'-----------------------------------------------------------------------------------------------'

 * [31  ] - enable (output tristated if 0)
 * [11:0] - value
```

Channels :

  * `0`: Clock tune ( low ) [ 12 bits ]
  * `1`: Clock tune ( high ) [ 12 bits ]
  * `2`: E1 RX Negative bias [ 8 bits ]
  * `3`: E1 RX Positive bias [ 8 bits ]
  * `4`: E1 RX Center Tap bias [ 8 bits ]
