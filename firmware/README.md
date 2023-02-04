Firmware Digi-Spherometer
=========================

The Firmware runs on an [ATTiny85]. The digital drop meter is connected to two
IO-pins, the [SSD1306 screen]/[SH1106 screen] is controlled via I²C.

See the [electronic](../pcb) section about the wiring.

### Install
To build, install the `avr-gcc` toolchain and `avrdude`. There is no
boot-loader, we directly program the chip with an AVR-compatible programmer
through the SPI in-circuit programming pins of the ATTiny85.

The first time the chip is programmed, make sure to set the fuses:
```bash
make fuse
```

Then, compiling and pushing new code to the ATTiny is as simple as invoking
`make flash`. There are some defines that you can pass directly on the
command-line, see [Makefile](./Makefile). In particular after
[calibration](../calibration), you might want to define the exact measured
spherometer radius.

```bash
make USER_DEFINES="-DSPHEROMETER_RADIUS_MM=50.2 -DALLOW_CONVEX_MEASUREMENTS=1" flash
```

The default compile works with the Autoutlet dial indicator. If you use a
Mitutoyo indicator, add `-DINDICATOR_MITUTOYO` to your `USER_DEFINES`.

These are the configuration options, settable via defines in the make commandline:

define                   | default value | Description
------------------------ |---------------|------------------------------------
DISP_SSD1306             | undefined     | Define if SSD1306 instead of SH1106 display is used
DISPLAY_I2C              | 0x78          | I²C address of display.
INDICATOR_DECIMALS       | 3             | Millimeter decimals supported. 3 means: resolution 1μm
INDICATOR_MITUTOYO       | undefined     | Read Mitutoyo instead of Autolet indicator
SPHEROMETER_RADIUS_MM    | 50.000        | Radius of spherometer ring, distance of balls to center.
SPHEROMETER_RADIUS_ERROR_MM | 0.1        | Accuracy of the spherometer radius. Influences error calculation
SPHEROMETER_FEET_BALL_DIAMETER_MM | 12.7 | Diameter of 'feet'-balls used for the ball correction.
TOOL_REFERENCE_FEATURE | undefined | If set to 1: Provide tool reference feature.
ALLOW_CONVEX_MEASUREMENTS| undefined     | If set to 1: Allow to measure also convex surfaces (shows radius negative and applies appropriate ball correction).
SCREENSAVER_SAMPLE_COUNT | undefined     | A number. If defined, turn off display after this number of samples with no change in measurement (good value would be around 1000). Use this for indicators such as Mitotoyo that don't auto-power off somewhat quickly. Reduces current consumption from ~15mA to ~4mA.

#### Mirror size choices for ƒ/N calculation
If you want to change the list of apertures to choose from for the
ƒ/N calculation (currently 6", 8", 10", 12" and 16") look for `aperture_items`
in `main.cc`.

### Interesting nitbits

#### Display and Font Handling
The ATTiny85 RAM is small (512 Byte), which does not allow for having a
frame-buffer for the 128x64 display (1KiB) in memory as many implementations do.
The fonts are compiled-in and directly copied from `PROGMEM`-memory.

The I²C and OLED display code is freshly implemented with only the relevant
features. The display-code only uses features from the SH1106 controller, so it
is compatible with SH1106 and SSD1306 (you still need a compile-time option
to distinguish the displays as there is a different pixel-offset).

The fonts were [generated as compact C-Arrays][bdfont.data] from bitmap BDF
fonts so that they can be compiled into the binary.

Fonts can be 'sparse' and only contain characters really needed in the
application. Also the encoding of each glyph is chosen to be optimal from a
choice of various run-length-encodings. This is necessary as flash-memory in
the ATTiny is pretty limited with 8KiB but we also use a fairly large font.
The [bdfont.data] project provides [Plane 0] UTF8 support, which makes
it easy to include special characters such as `ƒ`, `μ` or` ⚠`;
it is a separate project, check it out if you need font-support in your small
devices.

#### Error propagation
The error margins are done by having a (very simplistic) implementation of
a number data type and operators that do error propagation. That makes
the code very readable as the formula is written as if they were normal
numbers. But the whole chain of calculations are based on this `ErrorFloat`
type: the source of information, the diameter definition of the spherometer
and the values read from the dial indicator include the error margins, which
are then propagated through the spherometer calculation.

All calculations are all done with the `float` datatype;
even though this means somewhat larger code (as the ATTiny does not have a
floating point unit), it makes the code very readable.
Also: fixed point arithmetic would likely be somewhat complicated as we have
to deal with many order of magnitude range (squaring very large and very small
numbers).

#### Sleep
The device does not need an additional power button: it auto-detects
when the drop-indicator is switched off (because it stops sending updates), and
then goes to deep sleep; In that mode, the ATTiny only consumes < 0.8μA current
(plus some current drawn from the display in sleep mode).

The spherometer wakes up by registering a level interrupt on the indicator
clock line: as soon as it sees activity there, it comes out of sleep and
shows the relevant values.

#### Testing
After building with all the configurable parameters, there is also a program
called `host-testing` available, with all these parameters built-in for
testing on the local machine. This allows checking results without first
flashing to the microcontroller.

It takes any number of measurement values on the command line and outputs
the calculated:

```bash
./host-testing 0.7mm -0.5mm 0.02in
== Spherometer constants ==
Ball radius       :    6.350mm
Leg radius        :   50.000mm (-0.100mm/+0.100mm)
Assumed Dial error:    0.003mm

== Measurements ==
    0.700mm (-0.003mm/+0.003mm)  => 1779.714mm (-14.73mm/+14.87mm) convex radius
-   0.500mm (-0.003mm/+0.003mm)  => 2506.600mm (-24.84mm/+25.16mm) concave radius
    0.020in (-0.0001in/+0.0001in)  =>   96.635in (-0.954in/+0.966in) convex radius
```

![](../img/spherometer-devel.jpg)

[attiny85]: https://www.microchip.com/wwwproducts/en/ATtiny85
[ssd1306 screen]: https://www.ebay.com/sch/i.html?_nkw=ssd1306+i2c+128x64
[sh1106 screen]: https://www.ebay.com/sch/i.html?_nkw=sh1106+i2c+128x64
[Plane 0]: https://en.wikipedia.org/wiki/Plane_(Unicode)#Basic_Multilingual_Plane
[bdfont.data]: https://github.com/hzeller/bdfont.data
