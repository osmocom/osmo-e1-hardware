Mechanical Engineering Change Notes
===================================

Revision 0.0
------------

### Description

Original design

### Manufactured

* Two pairs of front/back panels were ordered at [Schaeffer](https://www.schaeffer-ag.de/en/) in
  1.5 mm thickness since 1.2 mm is not available.

* Back panel was cut at Makilab by @tnt .

### Issues noticed

After trying to fit the back panel, it became obvious that the calculations were off and the
height of the PCB (1.6 mm) was not properly accounted for, resulting in openings that were all
1.6 mm too low.

### Notes

* DXF files were not kept.
* Feeds & Speeds in the FreeCAD toolpaths were completely off and were overriden manually.


Revision 0.1
------------

### Description

First revision done after the 1.6 mm offset error was noticed. It was done even before the
rev 0.0 panels were back from Schaeffer.

### Changes compared to previous revision

* Moved all opening 1.6 mm up.
* Size of the 'mouse ears' (for square holes) has been reduced to 2 mm diameter.
* Also made sure each square opening is a single poly-line in the DXF.

### Manufactured

* Cut two pairs of front/back panels at Makilab for the two proto cases. One of the back
  panel was the previously 'bad' one (rev 0.0) and so the opening is now too wide and not
  looking great, but it fits and is functional.

### Issues noticed

* The SMA hole height is too low. Nominal height above PCB was 6.4mm but that's for the
  TE-Connectivity part. The SMA we're using is more like 6.85mm above the surface. To fix
  this, the hole will be changed to be taller than wide to accomodate both parts.

* All the square opening (single RJ45, dual RJ45, USB) have their excess located at the top
  and have their bottom flush with the expected pcb height. This is different from the circular
  opening which are aligned on their center. So change the positions so the excess clearance is
  evenly distributed top/bottom of the expected position.

# Notes

* Although new FrontPanelDesigner files were made for this revision, they were never
  submitted / manufactured.


Revision 1.0
------------

### Description

Production version, sent to manufacturer for production.

### Changes compared to previous revision

* Converted the SMA opening from a pure-circle to two half circles connected by lines.
  The upper half circle is 0.5 mm above the previous center

* Moved USB, 1xRJ45 and 2xRJ45 opening 0.2 mm down

### Manufactured

n/a

### Issues noticed

n/a
