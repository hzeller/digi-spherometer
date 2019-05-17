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

#include "sh1106-display.h"
#include "strfmt.h"

// Compiled-in fonts we're using for the UI.
#include "font-bignumber.h"
#include "font-smalltext.h"
#include "font-tinytext.h"

// The data to be filled by the indicator-reading function.
struct DialData {
  int32_t abs_value;  // Absolute value in 1/1000mm or 1/10000"
  bool negative;      // true if the value is negative.
  bool is_imperial;   // Reading is in imperial units
};

// The dial indicator header needs to provide this function.
// Given the clk_bit and data_bit to read from PINB, read the data and write
// the result to "data".
// Return 'true' if successful, 'false' if indicator is not sending data (i.e.
// is switched off).
static inline bool ReadDialIndicator(uint8_t clk_bit, uint8_t data_bit,
                                     DialData *data);

// Choose dial indicator implementation here.
#ifdef INDICATOR_MITUTOYO
#  include "dial-indicator-mitutoyo.h"
#else
#  include "dial-indicator-autoutlet.h"
#endif

/*
 * Choices of mirror diameters to be cylced through. In every context, there
 * are typically only a few common mirror sizes to deal with, so a single
 * button allows to cycle through them.
 * This can be shown in any unit as long as the actual value is stored in
 * millimeter for the calculation.
 *
 * All texts should have the same width.
 */
constexpr int kApertureChoices = 4;
struct {
  const char *text;
  const float mm;
} aperture_items[kApertureChoices] = {
  { "  ⌀6\"",  6*25.4 },
  { "  ⌀8\"",  8*25.4 },
  { " ⌀10\"", 10*25.4 },
  { " ⌀12\"", 12*25.4 },
};


// ------------------------------ configurable parameters ------------
// Distance center to feet. Radius of the Spherometer-feet circle.
constexpr float d_mm = 50.0f;

// Pins the dial indicator is connected to.
constexpr uint8_t CLK_BIT  = (1<<3);
constexpr uint8_t DATA_BIT = (1<<4);

constexpr uint8_t BUTTON_BIT = (1<<1);  // A button as UI input.

#ifndef INDICATOR_DECIMALS
#  define INDICATOR_DECIMALS 3
#endif

#if INDICATOR_DECIMALS == 3
constexpr float raw2mm = 1000.0f;
constexpr uint8_t raw_fmt_mm_digits = 3;
constexpr float raw2inch = 100000.0f;
constexpr uint8_t raw_fmt_inch_digits = 5;
#elif INDICATOR_DECIMALS == 2
constexpr float raw2mm = 100.0f;
constexpr uint8_t raw_fmt_mm_digits = 2;
constexpr float raw2inch = 10000.0f;
constexpr uint8_t raw_fmt_inch_digits = 4;
#else
#  error "Unhandled INDICATOR_DECIMALS value."
#endif

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

void ShowRadiusPage(SH1106Display *disp, const MeasureData &m,
                    uint8_t dia_choice) {
  uint8_t x;
  // Calculating the sag values to radius in their respective units.
  // We round the returned value to an integer, which is the type
  // we can properly string format below.
  // For imperial: fixpoint shift to display 1/10" unit
  const int32_t radius = roundf(m.imperial ? 10 * m.radius : m.radius);
  const bool is_overflow = (radius > 9999);   // Limit digits to screen-size
  const int32_t display_radius = is_overflow ? 9999 : radius;

  // -- Print focal length and ƒ/-Number
  if (is_overflow) {
    // Don't show any numbers in that case. Not every useful.
    for (int y = 0; y < 32; y += 8)
      disp->FillStripeRange(0, 127, y, 0x00);
    disp->Print(font_bignumber, 46, 0, "⚠");
  }
  else {
    const float f = m.radius / 2;  // Focal length of a sphere.
    // -- ƒ/-Number according to user choice from button.
    // f/5.43 ≈ ⌀ 6"
    const float f_mm = m.imperial ? 25.4 * f : f;
    const float dia = aperture_items[dia_choice].mm;
    const int32_t f_N = roundf(100 * f_mm / dia);  // 100* for extra digits
    x = disp->Print(font_smalltext, 0, 0, "ƒ/");
    x = disp->Print(font_smalltext, x, 0, strfmt(f_N, 2, 5));
    x = disp->Print(font_smalltext, x, 0, " ≈");
    // Ideally, we'd like to show this inverse to better draw the attention
    // to this number and the arrow-button right next to it.
    // However, it seems to suck brigthness out of that line which makes
    // it visually non-pleasing. TODO: Experiment with it once final
    // case with button is there.
    x = disp->Print(font_smalltext, x, 0, aperture_items[dia_choice].text,
                    false);

    // Print focal length in chosen units.
    constexpr int line2 = 16;
    x = disp->Print(font_smalltext, 0, line2, "ƒ=");
    const int32_t display_f = roundf(m.imperial ? 10*f : f);
    x = disp->Print(font_smalltext, x, line2,
                    strfmt(display_f, m.imperial ? 1:0, 5));
    x = disp->Print(font_smalltext, x, line2, m.imperial ? "\"  " : "mm");
  }

  // -- Printing the radius in a large font.

  // Make sure that it is clear we're talking about the sphere radius
  disp->Print(font_smalltext, 0, 48, "r=");

  // If the value is too large, we don't want to overflow the display.
  // Instead, we clamp it to highest value and show a little > indicator.
  if (is_overflow) {
    disp->Print(font_smalltext, 0, 32, ">");  // 'overflow' indicator
  } else {
    disp->Print(font_smalltext, 0, 32, " ");
  }

  // Different formatting of numbers in different units, including suffix
  if (m.imperial) {
    // One decimal point, total of 5 characters (including point) 999.9
    x = disp->Print(font_bignumber, 15, 32, strfmt(display_radius, 1, 5));
    disp->Print(font_bignumber, x, 32, "\"");
  } else {
    // No decimal point, total of 4 characters: 9999
    x = disp->Print(font_bignumber, 15, 32, strfmt(display_radius, 0, 4));
    disp->Print(font_smalltext, x, 48, "mm");
  }
}

// Returns if the absolute measurement value is deemed 'flat'. This should
// be around zero, but indicators can create a little bit of jumping around
// there. Let's be gentle and not drive OCD's crazy having to press the zero
// button all the time.
static inline bool is_flat(int32_t value) { return value < 2; }

int main() {
  PORTB = CLK_BIT | DATA_BIT;  // Pullup for the transistor level converter.
  _delay_ms(500);  // Let display warm up and get ready before the first i2c
  SH1106Display disp;
  Button button;

  DialData last_dial = {};
  uint8_t last_aperture_choice = 0xff;  // outside range, so guaranteed new

  uint8_t off_cycles = 0;
  uint8_t aperture_choice = 0;

  constexpr uint8_t kPowerOffAfterCycles = 50;
  constexpr uint8_t kStartShowingOutro = 4;
  DialData dial = {};
  for (;;) {
    const bool indicator_on = ReadDialIndicator(CLK_BIT, DATA_BIT, &dial);

    // If the dial indicator is off, we watch this for a while. After
    // kPowerOffAfterCycles, we go to deep sleep. In the time between
    // we detect the indicator to be off and going to sleep, we show the
    // github message on the display for people to find the project.
    if (!indicator_on)
      off_cycles++;
    else
      off_cycles = 0;

    if (off_cycles == kStartShowingOutro) {
      disp.ClearScreen();
      last_aperture_choice = 0xff;
      disp.Print(font_smalltext, 0, 0, "©Henner Zeller");
      disp.Print(font_tinytext, 0, 16, "github hzeller/");
      disp.Print(font_tinytext, 0, 32, "digi-spherometer");
      disp.Print(font_tinytext, 0, 48, "    GNU GPL");
    }
    else if (off_cycles > kPowerOffAfterCycles) {
      disp.SetOn(false);
      SleepTillDialIndicatorClocksAgain();
      // ... We're here after wakeup
      disp.Reset();      // Might've slept a long time. Make sure OK.
    }

    if (off_cycles)
      continue;

    if (button.clicked()) {
      aperture_choice += 1;
      if (aperture_choice >= kApertureChoices) aperture_choice = 0;
    }

    if (last_aperture_choice == 0xff
        || last_dial.negative != dial.negative
        || last_dial.is_imperial != dial.is_imperial
        || is_flat(last_dial.abs_value) != is_flat(dial.abs_value)) {
      disp.ClearScreen();  // Visuals will change. Clean-slatify.
    }

    if (is_flat(dial.abs_value)) {
      disp.Print(font_smalltext, 48, 0, "flat");
      disp.Print(font_bignumber, 34, 16, "OK");
    }
    else if (!dial.negative) {
      disp.Print(font_bignumber, 46, 0, "⚠");
      disp.Print(font_smalltext, 0, 32, "Please zero on");
      disp.Print(font_smalltext, 8, 48, "flat surface");
    }
    else if (dial.abs_value == last_dial.abs_value
             && dial.is_imperial == last_dial.is_imperial
             && last_aperture_choice == aperture_choice) {
      // Value or unit did not change. No need to update display.
    }
    else {
      // micrometer units or 0.00001" units. Imperial increments in steps of 5
      MeasureData prepared_data;
      const int32_t value = dial.abs_value;
      prepared_data.raw_sag = value;
      prepared_data.imperial = dial.is_imperial;
      const float sag = dial.is_imperial ? value / raw2inch : value / raw2mm;
      prepared_data.radius = calc_r(dial.is_imperial, sag);
      ShowRadiusPage(&disp, prepared_data, aperture_choice);
    }

    last_dial = dial;
    last_aperture_choice = aperture_choice;
  }
}
