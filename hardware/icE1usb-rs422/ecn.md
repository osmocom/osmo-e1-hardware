Electrical Engineering Change Notes
===================================

Revision 0.1
------------

### Description

First revision

### Manufactured

Boards ordered from Aisler on August 15th 2022.
Sent directly the Kicad files. See r0.1/ directory.

### Issues noticed

* Missing I2C pull-ups on the isolated side.

* The pinout for the external RJ45 is completely wrong.

* The 1k across 12V dissipates 144 mW. It's above the rating for
  the jelly-bean 0603 resistors I originally used and it also
  gets quite warm.

* The 120R termination resistors also can dissipate quite a bit
  of power. Technicaly up to 6V across leading to 300 mW !
  In our case, it's more like 3V so it's only 75 mW but still,
  some margin would be nice.

* The linear 5V reg does heat up a bit. Nothing critical, but
  could be improved.

### Notes

* All boards have been reworked to fix most pressing issues :
    * 3k3 pullups added on the isolated side.
    * Cut & manually rewired the external RJ45 pinout.
    * 1k across 12V switched to high power 250mW part.
    * 120R termination switched to high power 250mW part.

* Boards location:
    * One remained with tnt.
    * Two sent to LaF0rge. One will be installed in the datacenter
      to be used with the OCTOI hub, the other will remain for testing.


Revision 0.2
------------

### Description

Potential future version, if ever done ...

### TODO

* Add I2C pull ups on the isolated side.

* Fix the external RJ45 pinout.

* Use switchmode converter for supply. AP3211 looks like a good candidate.

* Replace the 1k resistor across 12V with a resistor + zener + depletion n-mos
  so that it appears as 1k when supply is < ~4V but currents limit above that.

* Use 0805 termination resistors maybe ? Or at least spec those as high power
  variants.
