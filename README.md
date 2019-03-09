A Digital Spherometer
=====================

Converting the sagittal measurement (SAG) from a dial indicator and converting
in your head to a sphere radius is a pain.

This project uses a microcontroller to make this more pleasent: it reads out
the digital indicator and converts the reading to a sphere radius value
printed on an OLED display. It also adds some other nifty features, such as
converting this into the ƒ/N for your mirror size.

Depending on the choice of inch/mm on the indicator, the value is automatically
shown in imperial or metric units. No additional buttons needed.

When the dial indicator is switched off, the μC detects that and goes to
deep sleep. Thus the devcie can be powered for long time on batteries in
stand-by.

The microcontroller is an [ATTiny85], the display is a [SSD1306]
compatible 128x64 OLED display with I²C interface. The dial indicator is
an generic [autoutlet-indicator] (very cheap in a get-what-you-pay-for sense:
It shows 1μm resolution, but the last digit is pretty much random. Next
experiments will be with a higher quality device...).

The microcontroller has less memory than would be needed to have a copy of
a framebuffer for that display (μC: 512 bytes; 64x128 display: 1024 bytes) which
made it more fun to hack. Writing text is done by sending compiled-in
fonts from PROGMEM to the display in multiple write operations.

The SPC connector for the indicator is 3D printed; the [EspDRO] project
has a [design for the plug] which works very well.

The [firmware](./firmware) can be compiled with an [avr-gcc].
The [frame](./frame) to hold the dial indicator, microcontroller+display and
batteries is still work in progress.

#### Developing on breadboard
![](img/spherometer-devel.jpg)

#### First prototype with rough mounting frame outline
![](img/spherometer-prototype.jpg)
[![](img/dial-case.png)](./frame)

[attiny85]: https://www.microchip.com/wwwproducts/en/ATtiny85
[ssd1306]: https://www.ebay.com/sch/i.html?_nkw=ssd1306+i2c+128x64
[autoutlet-indicator]: https://www.amazon.com/gp/product/B07C63VFN3
[EspDRO]: https://github.com/MGX3D/EspDRO
[design for the plug]: https://github.com/MGX3D/EspDRO/blob/master/CAD/spc_connector.scad
[avr-gcc]: https://gcc.gnu.org/wiki/avr-gcc