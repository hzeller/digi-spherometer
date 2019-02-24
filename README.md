A Digital Spherometer
=====================

Converting the SAG numbers read from a dial indicator and converting in
your head to a sphere radius is a pain (unless you don't mind squarig numbers
in your head and doing divisions with a bunch of decimals that is...).

This project uses a microcontroller to make this more pleasent: it reads out
the digital indicator and converts the reading to a sphere radius value
printed on an OLED display.

Depending on the choice of inch/mm on the indicator, the value is automatically
shown in imperial or metric units. No additional buttons needed.

When the dial indicator is switched off, the μC detects that and goes to
deep sleep. Thus the devcie can be powered for long time on batteries in
stand-by.

The microcontrollre used is a [ATTiny85], the display is a [SSD1306]
compatible 128x64 OLED display with I²C interface.

This microcontroller has less memory than would be needed to have a copy of
a framebuffer for that display (μC: 512 bytes; 64x128 display: 1024 bytes).
Writing text is done by sending compiled-in fonts from PROGMEM to the display
in multiple write operations.

![](img/spherometer-devel.jpg)

[attiny85]: https://www.microchip.com/wwwproducts/en/ATtiny85
[ssd1306]: https://www.ebay.com/sch/i.html?_nkw=ssd1306+i2c+128x64