Electrical Engineering Change Notes
===================================

Revision 0.0
------------

### Description

First revision

### Manufactured

Boards ordered from Aisler on August 12th 2020.
Gerbers used are in the repo.

### Issues noticed

* The E1 TX pulse were very weak. Probably would have worked but to rise the pulse amplitude
  to 1.5 V, the series resistance were lowered to 27R. (Original iCEbreaker proto had 33R on
  the icebreaker + 47R on the PMOD. The bitsy proto only had the 47R)

### Notes

* Kicad files were not kept since it's the same schematic and layout as rev 0.1
  with some value changed and all boards were either built with new values or reworked.


Revision 0.1
------------

### Description

This revision is mostly just to keep a record of the changes that were applied
directly on the r0.0 prototypes and how they were assembled.

### Changes compared to previous revisions

* E1 TX side resistors changed to 27R (from 82R)

* Consequently all other uses of 82R were updated
    * PLL filter switched to use 120R
    * Series resistor for clock switched to 27R (which is probably better)

* Connection from GND to USB shield changed to 91k (from 1k5)

* All 47R networks were swapped for 33R. 
    * New value was picked so that including the UP5k drive impedance (~20R),
      it matches 50R more closely for USB
    * Other places where it's used don't matter much.
    * E1 LED current is dominated by 595 output impedance anyway.
    * 33R is just as fine for serial termination/EMI isolation for GPS signals.

* Some silkscreen label was added on the PCB

### Manufactured

Rev 0.0 PCBs were used and assembled / reworked to match this revision.
Two boards in total: one for @LaF0rge and one for @tnt.

### Issues noticed / remaining

* Flash footprint is wrong. The selected flash is not using the "wide" variant of SO8.

* The NT/TE selection pin-header is completely wrong

* The lightpipe is a bit loose. Turns out the holes came out 10% oversized out of Aisler :/
  Dimensions left as the manufacturer ones and hope that the prod are closer to nominal.

* The E1 bias configuration should probably have been reversed with the `bias_fixed`
  assigned to the `_n` side. This way the variable bias would have been lower and the common
  mode voltage closer to what the comparator expect for LVDS.

  This however would break compatibility with the proto which is a waste and has no real-world
  impact. The common-mode voltage change is minimal and comparator work fine where it is. And
  even if it was an issue we could put the variable bias lower and flip the input polarity in
  the bitstream.


Revision 1.0
------------

### Description

Production version, sent to manufacturer for production.

### Changes compared to previous revisions

* Silkscreen updated

* Flash footprint updated to narrow SO8 (150mil)

* Rewired the NT/TE header properly

* Changed RF trackwidth to 0.175 mm to account for production stackup

* Changed board outline to cater to connectors being more outward
    * Extended by 0.5 mm on front/back but only in the center part of the PCB
      to avoid interfering wih the silicone ring
    * Reworked the 'tabs'
    * Adjusted the expected end panel position lines to account for 1 mm silicone thickness

* Adjust positions of connectors / ... 
    * Moved RGB led & lightpipe outward by 500u
    * Moved Serial connector outward by 500u
    * Moved USB outward by 500u
    * Moved GPIO RJ45 outward by 500u
    * Moved dual-RJ45 outward by 500u
    * Moved button outward by 750u

* As a result of those moves some minor adjustments were made
    * Copper fills extended by 500u on the left side + reworked upper corner
    * Copper fills extended by 500u on the right side
    * To avoid the small protrusions in the planes, vias in the RX CT bias were moved inward

* Rerouted 1v2 on the bottom to add more distance from GND vias. Also widened the
  T junction to one of the via to 0.6 mm

### Manufactured

* 52 units ( 26 panels ) were manufactured in Aug/Sep 2020 and all came back in perfect working
  order.

### Issues noticed

* PCB is slightly loose in the case because the case we used were slightly over size.
    * The "side tabs" could be made slightly larger (and possibly slight round curve) to
      have a bit more friction.
    * The "front/back" edge part that touches the silicone ring could be made a bit longer
      to compress the silicone a bit.
    * Those 2 would be very slight changes, like ~ 0.1mm on each side, still have to account
      for possible tolerance in the cases that could be smaller than nominal !

* The I2C connection to the GPS conflicts with usage for the extension port. Turns out the
  chinese GPS module act as a master on those lines.
  See https://projects.osmocom.org/issues/5664 for details of the issue.

### Notes

* Adding series resistor for `gps_reset_n` and `gps_pps` was considered, but deemed not necessary.
    * Traces are short, always close to ground plane and we have no strong RF/EMI emitter onboard.
    * `gps_pps` is generated by the module itself, so any fast edge is coming from the module.
    * `gps_reset_n` is static.

* Adding a capacitor between `tx_hi` and `tx_lo` (after the series resistor) was also an option.
  Testing showed it didn't really change / improve the pulse shape and simulation showed increased
  peak current (and thus stress on the UP5k drivers). I considered adding a DNP footprint but area
  was a bit cramped and some more work would have been needed for something that would most likely
  stay unpopulated.

* A thought that occured to me to improve pulse shape is to use an external buffered gate (those
  sot-23/sc-67 single/double gate buffer ICs) and possibly putting it in parallel :

```
From  -----,----------,--\/\/\/--->  To TX
FPGA        \        /     R         Magnetics
             \__|\__/
	        |/
```

  This would take strain off the FPGA driver, provide more current and the  delay of the gate
  might actually help make the pulse shape more correct.


Revision 1.1
------------

### Description

Updated production version with minor fixes and a few adaptations for further production runs
by sysmocom.

### Changes compared to previous revisions

* Updated to kicad 6 format
* Renumbered schematic pages
* Added NO jumpers on the I2C lines to GPS module
* Added fiducials
* Silkscreen updated

### Manufactured

* 105 units manufactured by sysmocom at the end of 2022.
