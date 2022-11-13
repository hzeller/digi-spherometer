# <h.zeller@acm.org>

# If the oled display is using a SSD1306 controller (typically the 0.96" are),
# then use this define
#DEFINES+=-DDISP_SSD1306

# I2C address of the display. 0x78 a common one.
#DEFINES+=-DDISPLAY_I2C=0x78

# By default, we assume a device with 3 decimals (0.001mm resolution). If it
# is a 2 digit device, change the following to 2
# (Note, you typically don't want a device with such rough resolution)
#DEFINES+=-DINDICATOR_DECIMALS=3

# If indicator is a Mitutoyo, add this
#DEFINES+=-DINDICATOR_MITUTOYO

# Radius of the spherometer ring in mm. In this project, this is by
# default 50mm, however after calibration you might want to tweak this value.
#DEFINES+=-DSPHEROMETER_RADIUS_MM=50.000

# Accuracy of the radius above. This is used for error propagation.
#DEFINES+=-DSPHEROMETER_RADIUS_ERROR_MM=0.1

# The diameter of the bearing balls we use as feet.
#DEFINES+=-DSPHEROMETER_FEET_BALL_DIAMETER_MM=12.7

# In case you always want to reference from a flat surface, you can disable
# the tool reference feature to avoid accidentally enabling it. In particular
# if you have a indicator that can keep an absolute position such as the
# Mitutoyo, you might not want this and simplify the user interface.
#
# Also if we allow convex measurements, this feature will be switched of for
# UI comlexity reasos.
#DEFINES+=-DDISABLE_TOOL_REFERENCE_FEATURE

# Allowing convex measurements means allowing for positive sag. This
# means the UI can't warn about non-zeroed range anymore, but it is more
# flexible.
#DEFINES+=-DALLOW_CONVEX_MEASUREMENTS

# Allow screen-saver to activate after this count of unchanged measurements.
# This switches the display off, preserving power.
# This is in particular useful for dial indicators that have a very long
# time before they go to auto-off, such as the Mitotoyo. The Autolet, however
# will auto-switch-off after a while so there is less of a need to have a
# screen-saver. The value represents number of measurements that read
# exactly the same. Valid range: 0..65535
#DEFINES+=-DSCREENSAVER_SAMPLE_COUNT=1000

# Defines can also be given on the command-line
DEFINES+=$(USER_DEFINES)

# Notes: to change using another architecture, e.g. some more beefy Atmega
# found in Arduinos
#   - change the TARGET_ARCH (e.g. atmega328 for a Arduino Nano)
#   - change the AVRDUDE_PART to the correct partnumber (e.g. m328p)
#   - choose the I2C_IMPL to be one of i2c-master-usi.o  or i2c-master-twi.o
#     (usi is used for attiny devices, twi for larger atmega devices)
TARGET_ARCH=-mmcu=attiny85
AVRDUDE_ARGS=-p t85 -c stk500v2
I2C_IMPL=i2c-master-usi.o
CPU_SPEED=8000000UL

# e.g. for Nano, this would be something like this
#TARGET_ARCH=-mmcu=atmega328
#AVRDUDE_ARGS=-p m328p -c arduino -b57600
#I2C_IMPL=i2c-master-twi.o
#CPU_SPEED=16000000UL

DEFINES+=-DF_CPU=$(CPU_SPEED)

CC=avr-gcc
CFLAGS=-Os -g -W -Wall -Wextra -fno-split-wide-types -fno-tree-scev-cprop -ffunction-sections -fdata-sections -mcall-prologues $(DEFINES) -I.
CXX=avr-g++
CXXFLAGS=$(CFLAGS) -std=c++11
AVRDUDE_DEVICE ?= /dev/ttyUSB0
AVRDUDE     = avrdude $(AVRDUDE_ARGS) -P $(AVRDUDE_DEVICE)
FLASH_CMD   = $(AVRDUDE) -e -U flash:w:main.hex
LINK=avr-g++ -g $(TARGET_ARCH) -Wl,-gc-sections,-relax

OBJECTS=main.o sh1106-display.o  strfmt.o \
	bdfont-support.o font-bignumber.o font-smalltext.o font-tinytext.o \
        $(I2C_IMPL)

all : main.hex

main.elf: $(OBJECTS)
	$(LINK) -o $@ $(OBJECTS)
	avr-size $@

disasm: main.elf
	avr-objdump -C -S main.elf

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex

main.cc : font-bignumber.c font-smalltext.c font-tinytext.c

# This requires http://github.com/hzeller/bdfont.data to be installed
# For distribution, we don't depend on %.chars, so that a git cloned project
# does not need bdfont-data-gen installed and compiles with provided files.
font-%.c: # %.chars
	bdfont-data-gen -s fonts/$*.bdf $* -C $*.chars

flash: main.hex
	$(FLASH_CMD)

clean:
	rm -f $(OBJECTS) main.elf main.hex

# Set to 8Mhz RC on Attiny85
fuse:
	$(AVRDUDE) -U lfuse:w:0xe2:m

# Make sure to recompile everything if cflags change.
compiler-flags: FORCE
	@echo '$(CXX) $(CXXFLAGS)' | cmp -s - $@ || echo '$(CXX) $(CXXFLAGS)' > $@

%.o : %.c compiler-flags
	$(CC) $(TARGET_ARCH) $(CFLAGS) -c -o $@ $<

%.o : %.cc compiler-flags
	$(CXX) $(TARGET_ARCH) $(CXXFLAGS) -c -o $@ $<

.PHONY: FORCE
