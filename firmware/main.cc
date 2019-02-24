// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2019 Henner Zeller <h.zeller@acm.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <util/delay.h>

#include "ssd1306-display.h"

#include "font-bignumber.h"
#include "font-smalltext.h"

constexpr float d_mm = 50.0f;

constexpr float d_inch = d_mm / 25.4f;
constexpr float d_mm_squared = d_mm * d_mm;
constexpr float d_inch_squared = d_inch * d_inch;

static float calc_r_mm(float sag) {
    return (d_mm_squared + sag*sag) / (2*sag);
}
static float calc_r_inch(float sag) {
    return (d_inch_squared + sag*sag) / (2*sag);
}

static constexpr uint8_t CLK_BIT  = (1<<4);
static constexpr uint8_t DATA_BIT = (1<<3);

struct DialData {
  int32_t value : 20;    // 20 bits of raw count data
  uint8_t negative : 1;  // negative if one.
  uint8_t off : 1;       // Not directly from
  uint8_t dummy : 1;
  uint8_t is_imperial : 1;
};

// Read the dial indicator. This is clocked out LSB.
// We get 24 bits, 20 of which are the read count (or number*2 for imperial,
// as they count the last half digit) and a negate and is_imperial bit.
// We also fill one 'off detection' bit: when we see that the dial is off.
// Since we're level converting the inputs with a transistor, the signals
// are inverted.
static DialData readDialIndicator() {
  constexpr uint16_t kInitialWaitTime = 5000;
  constexpr uint16_t kOffDetectionCount = 15000;

  union {
    struct DialData data;
    uint32_t bits;
  } result;
  result.bits = 0;

  // Wait until we have a stable time of non-clocking, so that we don't
  // jump into the middle of some clock cycle.
  uint16_t stable_clock_off_period = kInitialWaitTime;
  uint16_t clock_inactive_count = 0;
  while (stable_clock_off_period--) {
    if (PINB & CLK_BIT) {
      clock_inactive_count++;
      stable_clock_off_period = kInitialWaitTime;
    } else {
      clock_inactive_count = 0;
    }
    if (clock_inactive_count > kOffDetectionCount) {
      result.data.off = 1;
      return result.data;
    }
  }

  uint32_t current_bit = 1;
  clock_inactive_count = 0;
  for (int i = 0; i < 24; ++i) {
    while ((PINB & CLK_BIT) == 0)  // Wait for clk to go high.
      ;
    while (PINB & CLK_BIT) {      // Wait for negative edge
      clock_inactive_count++;
      if (clock_inactive_count > kOffDetectionCount) {
        result.data.off = 1;
        return result.data;
      }
    }
    if ((PINB & DATA_BIT) == 0)
      result.bits |= current_bit;
    current_bit <<= 1;
  }
  return result.data;
}

const char *strfmt(char *buffer, uint8_t buflen, int32_t v,
                   int8_t decimals, int8_t total_len = -1) {
    const bool is_neg = (v < 0);
    if (is_neg) v = -v;
    buffer = buffer + buflen - 1;
    *buffer-- = '\0';
    const char *pad_to = (total_len > 0) ? (buffer - total_len) : buffer;
    bool is_first = true;  // properly print and decimal-handling zero value.
    while (is_first || v > 0) {
        *buffer-- = (v % 10) + '0';
        v /= 10;
        if (--decimals == 0)
            *buffer-- = '.';
        is_first = false;
    }
    if (decimals > 0) {
        while (decimals-- > 0)
            *buffer-- = '0';
        *buffer-- = '.';
    }
    if (is_neg) *buffer-- = '-';
    while (pad_to < buffer)
        *buffer-- = ' ';
    return buffer + 1;
}

int main() {
    _delay_ms(100);  // Let display warm up and get ready before the first i2c
    SSD1306Display display;

    char buffer[16];
    uint8_t x;
    DialData last_dial_data;
    for (;;) {
      DialData dial_data = readDialIndicator();

      if (last_dial_data.off != dial_data.off
          || last_dial_data.negative != dial_data.negative
          || last_dial_data.is_imperial != dial_data.is_imperial
          || (last_dial_data.value == 0) != (dial_data.value == 0)) {
        display.ClearScreen();  // Visuals will change. Clean-slatify.
      }
      last_dial_data = dial_data;

      if (dial_data.off) {
        // TODO: go to sleep.
        display.WriteString(&progmem_font_smalltext.meta, 16, 0,
                            "-------------");
      }
      else if (dial_data.value == 0) {
        display.WriteString(
          &progmem_font_bignumber.meta, 0, 16, "0000");  // "flat"
      }
      else if (!dial_data.negative) {
        display.WriteString(&progmem_font_smalltext.meta, 16, 0,
                            "-Z0-"); // "Please zero on flat surface"
      }
      else {
        int32_t value = dial_data.value;
        if (dial_data.is_imperial) value *= 5;

        // Print read value
        x = display.WriteString(&progmem_font_smalltext.meta, 0, 0, "sag=");
        x = display.WriteString(&progmem_font_smalltext.meta, x, 0,
                                strfmt(buffer, sizeof(buffer),
                                       value,
                                       dial_data.is_imperial ? 5 : 3, 10));
        display.WriteString(&progmem_font_smalltext.meta, x, 0,
                            dial_data.is_imperial ? "\"  " : "mm");

        float sag = dial_data.is_imperial ? value / 100000.0f : value / 1000.0f;
        if (dial_data.is_imperial) {
          int32_t radius = 10 * calc_r_inch(sag);
          if (radius > 9999) {   // Limit digits to screen-size
            display.WriteString(&progmem_font_smalltext.meta, 0, 16, "m");
            radius = 9999;
          } else {
            display.WriteString(&progmem_font_smalltext.meta, 0, 16, " ");
          }
          x = display.WriteString(
            &progmem_font_bignumber.meta, 12, 16,
            strfmt(buffer, sizeof(buffer), radius, 1, 5));
          display.WriteString(&progmem_font_smalltext.meta, x, 16, "\"");
        }
        else {
          int32_t radius = calc_r_mm(sag);
          if (radius > 9999) { // Limit digits to screen-size
            display.WriteString(&progmem_font_smalltext.meta, 0, 16, "m");
            radius = 9999;
          } else {
            display.WriteString(&progmem_font_smalltext.meta, 0, 16, " ");
          }
          x = display.WriteString(
            &progmem_font_bignumber.meta, 12, 16,
            strfmt(buffer, sizeof(buffer), radius, 0, 4));
          display.WriteString(&progmem_font_smalltext.meta, x, 40, "mm");
        }
      }
    }
}
