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
constexpr uint8_t CLK_BIT  = (1<<4);
constexpr uint8_t DATA_BIT = (1<<3);
constexpr uint8_t BUTTON_BIT = (1<<1);

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

// Keep track of button release before next clicked() event is triggered.
class Button {
public:
  Button() { PORTB |= BUTTON_BIT; /* pullup */}

  bool clicked() {
    const bool current_press = (PINB & BUTTON_BIT) == 0;  // negative logic
    const bool before = previous_pressed_;
    previous_pressed_ = current_press;
    return (current_press && !before);
  }

private:
  bool previous_pressed_ = false;
};

// Precalculated measure data, to be used in radius and focal page.
struct MeasureData {
  uint32_t raw_sag;
  uint8_t imperial;
  float radius;
};

void ShowRadiusPage(SSD1306Display *disp, const MeasureData &m) {
  // Print sag value we got from the dial indicator
  uint8_t x = disp->Print(font_smalltext, 0, 0, "sag=");
  x = disp->Print(font_smalltext, x, 0,
                  strfmt(m.raw_sag, m.imperial ? 5 : 3, 7));
  disp->Print(font_smalltext, x, 0, m.imperial ? "\"  " : "mm");

  // Make sure that it is clear we're talking about the sphere radius
  disp->Print(font_smalltext, 0, 40, "r=");

  // Calculating the sag values to radius in their respective units.
  // We round the returned value to an integer, which is the type
  // we can properly string format below.
  // For imperial: fixpoint shift to display 1/10" unit
  int32_t radius = roundf(m.imperial ? 10 * m.radius : m.radius);

  // If the value is too large, we don't want to overflow the display.
  // Instead, we clamp it to highest value and show a little > indicator.
  if (radius > 9999) {   // Limit digits to screen-size
    disp->Print(font_smalltext, 0, 24, ">");
    radius = 9999;
  } else {
    disp->Print(font_smalltext, 0, 24, " ");
  }

  // Different formatting of numbers in different units, including suffix
  if (m.imperial) {
    // One decimal point, total of 5 characters (including point) 999.9
    x = disp->Print(font_bignumber, 15, 24, strfmt(radius, 1, 5));
    disp->Print(font_bignumber, x, 16, "\"");
  } else {
    // No decimal point, total of 4 characters: 9999
    x = disp->Print(font_bignumber, 15, 24, strfmt(radius, 0, 4));
    disp->Print(font_smalltext, x, 40, "mm");
  }
}

void ShowFocusPage(SSD1306Display *disp, const MeasureData &m, int page) {
  uint8_t x;
  const float f = m.radius / 2;
  x = disp->Print(font_smalltext, 0, 0, "ƒ = ");
  const int32_t display_f = roundf(m.imperial ? 10*f : f);
  x = disp->Print(font_smalltext, x, 0, strfmt(display_f, m.imperial ? 1 : 0));
  x = disp->Print(font_smalltext, x, 0, m.imperial ? "\"  " : "mm");
  disp->FillStripeRange(x, 127, 0, 0x00);
  disp->FillStripeRange(x, 127, 8, 0x00);

  constexpr uint8_t per_page = 3;
  // First in mm, second in inches.
  float sample_diameters[][6] = {{ 150, 200, 250,    300, 400, 600 },
                                 { 6, 8, 10,         12, 16, 20 }};
  for (uint8_t i = 0; i < per_page; ++i) {
    const float dia = sample_diameters[m.imperial][i + per_page*page];
    const int32_t f_N = roundf(10 * f / dia);  // 10* for extra digit
    const uint8_t y = 16 + i*16;
    x = disp->Print(font_smalltext, 0, y, strfmt(dia, 0, 2));
    x = disp->Print(font_smalltext, x, y, m.imperial ? "\" ≈ ƒ/" : "mm ≈ ƒ/");
    x = disp->Print(font_smalltext, x, y, strfmt(f_N, 1));
    disp->FillStripeRange(x, 127, y, 0x00);
    disp->FillStripeRange(x, 127, y + 8, 0x00);
  }

  // Show 'scrollbar'. We have two pages, so show a bright bar going down.
  for (int i = 0; i < 4; ++i) {
    disp->FillStripeRange(127, 128, (i+4*page)*8, 0xff);
  }
}

int main() {
  _delay_ms(500);  // Let display warm up and get ready before the first i2c
  SSD1306Display disp;
  Button button;

  DialData last_dial;
  uint8_t last_page = 0xff;  // outside range, so guaranteed not seen before.

  uint32_t off_cycles = 0;
  uint8_t display_page = 0;

  constexpr uint32_t kPowerOffAfterCycles = 150;
  for (;;) {
    DialData dial = ReadDialIndicator(CLK_BIT, DATA_BIT);

    if (button.clicked()) {
      display_page += 1;
      if (display_page == 3) display_page = 0;
      disp.ClearScreen();
    }

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
      disp.Reset();      // Might've slept a long time. Make sure OK.
      last_page = 0xff;
      display_page = 0;  // Go back to radius page after waking up.
    }

    if (last_dial.off != dial.off
        || last_dial.negative != dial.negative
        || last_dial.is_imperial != dial.is_imperial
        || (last_dial.value == 0) != (dial.value == 0)) {
      disp.ClearScreen();  // Visuals will change. Clean-slatify.
    }

    if (dial.off) {
      if (!last_dial.off) {  // Only need to write if we just got here.
        disp.Print(font_smalltext, 0, 0, "© Henner Zeller");
        disp.Print(font_tinytext, 0, 16, "GNU Public License");
        disp.Print(font_tinytext, 0, 32, "github.com/hzeller/");
        disp.Print(font_tinytext, 0, 48, "digi-spherometer");
      }
    }
    else if (dial.value == 0) {
      disp.Print(font_smalltext, 48, 0, "flat");
      disp.Print(font_okfont, 34, 16, "OK");
    }
    else if (!dial.negative) {
      disp.Print(font_smalltext, 0, 8, "Please zero on");
      disp.Print(font_smalltext, 8, 32, "flat surface");
    }
    else if (dial.value == last_dial.value
             && dial.is_imperial == last_dial.is_imperial
             && last_page == display_page) {
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

      if (display_page == 0)
        ShowRadiusPage(&disp, prepared_data);
      else
        ShowFocusPage(&disp, prepared_data, display_page - 1);
    }

    last_dial = dial;
    last_page = display_page;
  }
}
