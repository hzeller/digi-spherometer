Calibration
==========

Any mechanical system build to dimensions will be slightly off from the desired
dimensions.  These inaccuracies might be small (if a CNC machine was used) or
larger if hand-manufacturing was done. If we measure the final outcome, we can
correct for that.

The dimension that is critical for correct measurements it the distance
of the dial indicator tip to the edge of the circle spanned by the three
balls on the base-plate. Many spherometer building instructions are only
concerned about measuring the radius of the circle spanned by the balls, but
that doesn't help if the indicator tip is off-center. Here, we take all these
into account.

For our measurement, we'll use an optical tool: a flatbed scanner. It has
a pretty accurate fixed pixel-to-realworld resolution which allows us to
determine accurate absolute measurements. Given the high resolution, the
accuracy can be in the order of 1/100mm.

# Scanning

Put your final spherometer on the bed and create a high-resolution scan
(>= 600dpi). Be aware that the resulting images are _very_ large, so make sure
to select only the relevant part in the preview.

![](../img/calibration-scan.jpg)

It might be a little hard later to find the exact center where the ball hits
the surface. Maybe experimenting putting the balls in a little puddle of ink
might be a good idea.

# Measuring

Load the image into your favorite image editing program (I use [gimp]), and
zoom into the relevant parts and hover the mouse-cursor right where you see
the center of the ball. Write down the coordinate where that is (in gimp, it
shows in the status line at the bottom).

Do this for all three balls, then the center and write these coordinates
to a file. These coordinates can be quite large numbers as the scan has a
pretty high resolution.

All coordinates are written to a text-file; here are the are numbers for my
spherometer scanned with 2400dpi, written to a textfile `positions.txt`.

```
4782 1288
1394 8776
9587 7980
5292 6033
```

# Calculating and updating firmware

Now, we want to convert these values to something useful to plug into our
firmware.

For that, there is a little program in this directory you can use. Build it:

```
make
```

Now, we can use the resulting `spcalc` program. We just give it the coordinates
from above on stdin, and also tell the program that the resolution was in
2400dpi:


```
./spcalc 2400 < positions.txt
Circle radius...............: 50.285mm
Indicator to true center....:  0.379mm
Effective Spherometer radius: 49.906mm
```

Done. Now we can build the firmware with the exact value for more exact
readings:

```
cd ../firmware
make USER_DEFINES=-DSPHEROMETER_RADIUS_MM=49.906
```

See the [firmware-page](../firmware) for building and flashing instructions.

[gimp]: https://www.gimp.org/
