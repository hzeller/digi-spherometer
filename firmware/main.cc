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
#include <math.h>

#include "ssd1306-display.h"
#include "strfmt.h"

// Compiled-in fonts we're using for the UI.
#include "font-bignumber.h"
#include "font-smalltext.h"
#include "font-tinytext.h"
#include "font-okfont.h"

// Choose reading function of dial indicator here.
// (only  indicator we support right now is autolet, but we could have a
// conditional include here)
#include "autolet-dial-indicator.h"

// ------------------------------ configurable parameters ------------
// Distance center to feet. Radius of the Spherometer-feet circle.
constexpr float d_mm = 50.0f;

// Pins the dial indicator is connected to.
static constexpr uint8_t CLK_BIT  = (1<<4);
static constexpr uint8_t DATA_BIT = (1<<3);

// TODO: also take radius of balls used as feet into account.
// TODO: factors and decimals for 2 and 3 digit indicators
// ------------------------------ nothing to be changed below --------
// ... derived from the above; let's compile-time calculate them.
constexpr float d_inch = d_mm / 25.4f;
constexpr float d_mm_squared = d_mm * d_mm;
constexpr float d_inch_squared = d_inch * d_inch;

static float calc_r(bool is_imperial, float sag) {
  return is_imperial
    ? ((d_inch_squared + sag*sag) / (2*sag))
    : ((d_mm_squared + sag*sag) / (2*sag));
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

struct MeasureData {
  uint32_t raw_sag;
  uint8_t imperial;
  float radius;
};

void ShowRadiusPage(SSD1306Display *disp, struct MeasureData m) {
  char buffer[16];
  // Print sag value we got from the dial indicator
  uint8_t x = disp->Print(&progmem_font_smalltext.meta, 0, 0, "sag=");
  x = disp->Print(&progmem_font_smalltext.meta, x, 0,
                  strfmt(buffer, sizeof(buffer),
                         m.raw_sag, m.imperial ? 5 : 3, 7));
  disp->Print(&progmem_font_smalltext.meta, x, 0, m.imperial ? "\"  " : "mm");

  // Make sure that it is clear we're talking about the sphere radius
  disp->Print(&progmem_font_smalltext.meta, 0, 40, "r=");

  // Calculating the sag values to radius in their respective units.
  // We roundthe returned value to an integer, which is the type
  // we can properly string format below.
  // Fixpoint shift to display 1/10" unit
  int32_t radius = roundf(m.imperial ? 10 * m.radius : m.radius);

  // If the value is too large, we don't want to overflow the display.
  // Instead, we clamp it to highest value and show a little > indicator.
  if (radius > 9999) {   // Limit digits to screen-size
    disp->Print(&progmem_font_smalltext.meta, 0, 24, ">");
    radius = 9999;
  } else {
    disp->Print(&progmem_font_smalltext.meta, 0, 24, " ");
  }

  // Different formatting of numbers in different units, including suffix
  if (m.imperial) {
    // One decimal point, total of 5 characters (including point) 999.9
    const char *str = strfmt(buffer, sizeof(buffer), radius, 1, 5);
    x = disp->Print(&progmem_font_bignumber.meta, 15, 24, str);
    disp->Print(&progmem_font_bignumber.meta, x, 16, "\"");
  } else {
    // No decimal point, total of 4 characters: 9999
    const char *str = strfmt(buffer, sizeof(buffer), radius, 0, 4);
    x = disp->Print(&progmem_font_bignumber.meta, 15, 24, str);
    disp->Print(&progmem_font_smalltext.meta, x, 40, "mm");
  }
}

int main() {
  _delay_ms(500);  // Let display warm up and get ready before the first i2c
  SSD1306Display disp;

  DialData last_dial;
  uint32_t off_cycles = 0;
  constexpr uint32_t kPowerOffAfterCycles = 150;
  for (;;) {
    DialData dial = ReadDialIndicator(CLK_BIT, DATA_BIT);

    // If the dial indicator is off, we watch this for a while. After
    // kPowerOffAfterCycles, we go to deep sleep. In the time between
    // we detect the indicator to be off and going to sleep, we show the
    // github message on the display for people to find.
    if (dial.off)
      off_cycles++;
    else
      off_cycles = 0;

    if (off_cycles > kPowerOffAfterCycles) {
      disp.SetOn(false);
      SleepTillDialIndicatorClocksAgain();
      disp.Reset();   // Might've slept a long time. Make sure OK.
    }

    if (last_dial.off != dial.off
        || last_dial.negative != dial.negative
        || last_dial.is_imperial != dial.is_imperial
        || (last_dial.value == 0) != (dial.value == 0)) {
      disp.ClearScreen();  // Visuals will change. Clean-slatify.
    }

    if (dial.off) {
      if (!last_dial.off) {  // Only need to write if we just got here.
        disp.Print(&progmem_font_smalltext.meta, 0, 0, "© Henner Zeller");
        disp.Print(&progmem_font_tinytext.meta, 0, 16, "GNU Public License");
        disp.Print(&progmem_font_tinytext.meta, 0, 32, "github.com/hzeller/");
        disp.Print(&progmem_font_tinytext.meta, 0, 48, "digi-spherometer");
      }
    }
    else if (dial.value == 0) {
      disp.Print(&progmem_font_smalltext.meta, 48, 0, "flat");
      disp.Print(&progmem_font_okfont.meta, 34, 16, "OK");
    }
    else if (!dial.negative) {
      disp.Print(&progmem_font_smalltext.meta, 0, 8, "Please zero on");
      disp.Print(&progmem_font_smalltext.meta, 8, 32, "flat surface");
    }
    else if (dial.value == last_dial.value
             && dial.is_imperial == last_dial.is_imperial) {
      // Value or unit did not change. No need to update display.
    }
    else {
      // micrometer units or 0.00001" units. Imperial increments in steps of 5
      struct MeasureData prepared_data;
      int32_t value = dial.is_imperial ? dial.value * 5 : dial.value;
      prepared_data.raw_sag = value;
      prepared_data.imperial = dial.is_imperial;
      const float sag = dial.is_imperial ? value / 100000.0f : value / 1000.0f;
      prepared_data.radius = calc_r(dial.is_imperial, sag);

      ShowRadiusPage(&disp, prepared_data);
    }
    last_dial = dial;
  }
}
