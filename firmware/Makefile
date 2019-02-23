# <h.zeller@acm.org>
DEFINES=-DF_CPU=8000000UL

TARGET_ARCH=-mmcu=attiny85
CC=avr-gcc
CFLAGS=-Os -g -Wall -Wextra -ffunction-sections -fdata-sections -mcall-prologues $(DEFINES) -I.
CXX=avr-g++
CXXFLAGS=$(CFLAGS) -std=c++11
AVRDUDE_DEVICE ?= /dev/ttyUSB0
AVRDUDE     = avrdude -p t85 -c stk500v2 -P $(AVRDUDE_DEVICE)
FLASH_CMD   = $(AVRDUDE) -e -U flash:w:main.hex
LINK=avr-g++ -g $(TARGET_ARCH) -Wl,-gc-sections
OBJECTS=main.o ssd1306-display.o tiny-i2c-master.o \
        font-bignumber.o font-smalltext.o

all : main.hex

main.elf: $(OBJECTS)
	$(LINK) -o $@ $(OBJECTS)
	avr-size $@

disasm: main.elf
	avr-objdump -C -S main.elf

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex

flash: main.hex
	$(FLASH_CMD)

clean:
	rm -f $(OBJECTS) main.elf main.hex

# Set to 8Mhz RC
fuse:
	$(AVRDUDE) -U lfuse:w:0xe2:m
