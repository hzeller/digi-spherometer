# <h.zeller@acm.org>
DEFINES=-DF_CPU=8000000UL

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

# Defines can also be given on the command-line
DEFINES+=$(USER_DEFINES)

TARGET_ARCH=-mmcu=attiny85
CC=avr-gcc
CFLAGS=-Os -g -W -Wall -Wextra -ffunction-sections -fdata-sections -mcall-prologues $(DEFINES) -I.
CXX=avr-g++
CXXFLAGS=$(CFLAGS) -std=c++11
AVRDUDE_DEVICE ?= /dev/ttyUSB0
AVRDUDE     = avrdude -p t85 -c stk500v2 -P $(AVRDUDE_DEVICE)
FLASH_CMD   = $(AVRDUDE) -e -U flash:w:main.hex
LINK=avr-g++ -g $(TARGET_ARCH) -Wl,-gc-sections
OBJECTS=main.o sh1106-display.o tiny-i2c-master.o strfmt.o \
	bdfont-support.o font-bignumber.o font-smalltext.o font-tinytext.o

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
