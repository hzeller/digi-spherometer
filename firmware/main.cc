// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// Copyright (C) 2019 Henner Zeller <h.zeller@acm.org>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <util/delay.h>

#include "ssd1306-display.h"
#include "strfmt.h"

// Compiled-in fonts we're using for the UI.
#include "font-bignumber.h"
#include "font-smalltext.h"
#include "font-tinytext.h"

// Distance center to feet. Radius of the Spherometer-feet circle.
constexpr float d_mm = 50.0f;

// TODO: also take radius of balls used as feet into account.

// ... derived from the above; let's compile-time calculate them.
constexpr float d_inch = d_mm / 25.4f;
constexpr float d_mm_squared = d_mm * d_mm;
constexpr float d_inch_squared = d_inch * d_inch;

static float calc_r_mm(float sag) {
    return (d_mm_squared + sag*sag) / (2*sag);
}
static float calc_r_inch(float sag) {
    return (d_inch_squared + sag*sag) / (2*sag);
}

// Pins the dial indicator is connected to.
static constexpr uint8_t CLK_BIT  = (1<<4);
static constexpr uint8_t DATA_BIT = (1<<3);

struct DialData {
  int32_t value : 20;      // 20 bits of raw count data
  uint8_t negative : 1;    // if true, value avoe is a negative number.
  uint8_t off : 1;         // dial indicator off, couldn't read data.
  uint8_t dummy : 1;
  uint8_t is_imperial : 1;  // Reading is in imperial units
};

// Read the dial indicator (my model: AUTOLET digital indicator with 1μm res).
//
// Data is clocked out LSB first. Data is read on rising edge of CLK.
// We get 24 bits, 20 of which are the read count (or number*2 for imperial,
// as they count the last half digit) and a negate and is_imperial bit.
// We also fill one 'off detection' bit: when we see that the dial is not
// clocking, so we can consider it OFF (otherwise we'd wait forever here).
//
// Since we're level converting the inputs from 1.5V to VCC with a transistor,
// the signals are inverted, i.e. we're looking at the _falling_ edge of
// CLK and invert the bit we read.
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

// Get microcontroller to deep sleep. We enable a level changing interrupt
// to come back online. We are listening on the clock line of the dial
// indicator for that. That way, we don't need any button.
EMPTY_INTERRUPT(PCINT0_vect);
static void SleepTillDialIndicatorClocksAgain() {
  cli();
  GIMSK |= (1<<PCIE);   // level change interrupt
  PCMSK = CLK_BIT;      // Switch on level detection on the CLK of indicator
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  sleep_enable();
  sei();
  sleep_cpu();

  // Zzzz...

  // Waking up due to interrupt.
  sleep_disable();
  GIMSK = 0;
}

int main() {
    _delay_ms(500);  // Let display warm up and get ready before the first i2c
    SSD1306Display display;

    char buffer[16];
    uint8_t x;
    DialData last_dial_data;
    uint32_t off_cycles = 0;
    constexpr uint32_t kPowerOffAfterCycles = 150;
    for (;;) {
      DialData dial_data = readDialIndicator();

      // If the dial indicator is off, we watch this for a while. After
      // kPowerOffAfterCycles, we go to deep sleep. In the time between
      // we detect the indicator to be off and going to sleep, we show the
      // github message on the display for people to find.
      if (dial_data.off)
        off_cycles++;
      else
        off_cycles = 0;

      if (off_cycles > kPowerOffAfterCycles) {
        display.SetOn(false);
        SleepTillDialIndicatorClocksAgain();
        display.Reset();   // Might've slept a long time. Make sure OK.
      }

      if (last_dial_data.off != dial_data.off
          || last_dial_data.negative != dial_data.negative
          || last_dial_data.is_imperial != dial_data.is_imperial
          || (last_dial_data.value == 0) != (dial_data.value == 0)) {
        display.ClearScreen();  // Visuals will change. Clean-slatify.
      }

      if (dial_data.off) {
        if (!last_dial_data.off) {  // Only need to write if we just got here.
          display.WriteString(&progmem_font_smalltext.meta, 0, 0,
                              "© Henner Zeller");
          display.WriteString(&progmem_font_tinytext.meta, 0, 16,
                              "GNU Public License");
          display.WriteString(&progmem_font_tinytext.meta, 0, 32,
                              "github.com/hzeller/");
          display.WriteString(&progmem_font_tinytext.meta, 0, 48,
                              "digi-spherometer");
        }
      }
      else if (dial_data.value == 0) {
        display.WriteString(&progmem_font_smalltext.meta, 48, 0,
                            "flat");
        display.WriteString(
          &progmem_font_bignumber.meta, 34, 16, "OK");
      }
      else if (!dial_data.negative) {
        display.WriteString(&progmem_font_smalltext.meta, 0, 8,
                            "Please zero on");
        display.WriteString(&progmem_font_smalltext.meta, 8, 32,
                            "flat surface");
      }
      else if (dial_data.value == last_dial_data.value
               && dial_data.is_imperial == last_dial_data.is_imperial) {
        // Value or unit did not change. No need to update display.
      }
      else {
        int32_t value = dial_data.value;         // micrometer units
        if (dial_data.is_imperial) value *= 5;   // 0.00001" units.

        // Print sag value we got from the dial indicator
        x = display.WriteString(&progmem_font_smalltext.meta, 0, 0, "sag=");
        x = display.WriteString(&progmem_font_smalltext.meta, x, 0,
                                strfmt(buffer, sizeof(buffer), value,
                                       dial_data.is_imperial ? 5 : 3, 7));
        display.WriteString(&progmem_font_smalltext.meta, x, 0,
                            dial_data.is_imperial ? "\"  " : "mm");

        // Make sure that it is clear we're talking about the sphere radius
        display.WriteString(&progmem_font_smalltext.meta, 0, 40, "r=");

        // Calculating the sag values to radius in their respective units.
        // We truncate the returned value to an integer, which is the type
        // we can properly string format below.
        float sag = dial_data.is_imperial ? value / 100000.0f : value / 1000.0f;
        int32_t radius = dial_data.is_imperial
          ? 10 * calc_r_inch(sag)   // Fixpoint shift to display 1/10" unit
          : calc_r_mm(sag);

        // If the value is too large, we don't want to overflow the display.
        // Instead, we clamp it to highest value and show a little > indicator.
        if (radius > 9999) {   // Limit digits to screen-size
          display.WriteString(&progmem_font_smalltext.meta, 0, 24, ">");
          radius = 9999;
        } else {
          display.WriteString(&progmem_font_smalltext.meta, 0, 24, " ");
        }

        // Different formatting of numbers in different units, including suffix
        if (dial_data.is_imperial) {
          // One decimal point, total of 5 characters (including point) 999.9
          const char *str = strfmt(buffer, sizeof(buffer), radius, 1, 5);
          x = display.WriteString(&progmem_font_bignumber.meta, 15, 24, str);
          display.WriteString(&progmem_font_bignumber.meta, x, 16, "\"");
        }
        else {
          // No decimal point, total of 4 characters: 9999
          const char *str = strfmt(buffer, sizeof(buffer), radius, 0, 4);
          x = display.WriteString(&progmem_font_bignumber.meta, 15, 24, str);
          display.WriteString(&progmem_font_smalltext.meta, x, 40, "mm");
        }
      }
      last_dial_data = dial_data;
    }
}
