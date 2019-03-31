# <h.zeller@acm.org>
DEFINES=-DF_CPU=8000000UL

# If the oled display is using a SH1106 controller (typically the 1.3" are),
# then uncomment the following define
DEFINES+=-DDISP_SH1106

TARGET_ARCH=-mmcu=attiny85
CC=avr-gcc
CFLAGS=-Os -g -Wall -Wextra -ffunction-sections -fdata-sections -mcall-prologues $(DEFINES) -I.
CXX=avr-g++
CXXFLAGS=$(CFLAGS) -std=c++11
AVRDUDE_DEVICE ?= /dev/ttyUSB0
AVRDUDE     = avrdude -p t85 -c stk500v2 -P $(AVRDUDE_DEVICE)
FLASH_CMD   = $(AVRDUDE) -e -U flash:w:main.hex
LINK=avr-g++ -g $(TARGET_ARCH) -Wl,-gc-sections
OBJECTS=main.o ssd1306-display.o tiny-i2c-master.o strfmt.o \
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

# this requires http://github.com/hzeller/bdfont.data to be installed
# For distribution, we don't depend on %.chars, so that a git cloned
# project does not need bdfont-data-gen installed.
font-%.c:
	bdfont-data-gen -s fonts/$*.bdf $* -C $*.chars

flash: main.hex
	$(FLASH_CMD)

clean:
	rm -f $(OBJECTS) main.elf main.hex

# Set to 8Mhz RC
fuse:
	$(AVRDUDE) -U lfuse:w:0xe2:m
