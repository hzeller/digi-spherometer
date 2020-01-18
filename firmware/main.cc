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
#include <math.h>
#include <stdint.h>
#include <util/delay.h>

#include "sh1106-display.h"
#include "strfmt.h"
#include "error-float.h"

// Compiled-in fonts we're using for the UI.
#include "font-bignumber.h"
#include "font-smalltext.h"
#include "font-tinytext.h"

// ------------------------------ configurable parameters ------------
/*
 * Choices of mirror diameters to be cylced through. In every workshop, there
 * are typically only a few common mirror sizes to deal with, so a single
 * button allows to cycle through them.
 * There is a display text which can show any text or unit, while the actual
 * value is stored in millimeter for the calculation.
 *
 * All texts should have the same width.
 */
static constexpr int kApertureChoices = 5;
static constexpr struct {
  const char *text;
  const float mm;
} aperture_items[kApertureChoices] = {
  { "   ⌀6\"",  6*25.4 },
  { "   ⌀8\"",  8*25.4 },
  { "  ⌀10\"", 10*25.4 },
  { "  ⌀12\"", 12*25.4 },
  { "  ⌀16\"", 16*25.4 },
};

// Distance center to feet. Radius of the Spherometer-feet circle.
#ifndef SPHEROMETER_RADIUS_MM
#  define SPHEROMETER_RADIUS_MM 50
#endif
#ifndef SPHEROMETER_RADIUS_ERROR_MM
#  define SPHEROMETER_RADIUS_ERROR_MM 0.1
#endif
static constexpr ErrorFloat d_mm(SPHEROMETER_RADIUS_MM,
                                 SPHEROMETER_RADIUS_ERROR_MM);

#ifndef SPHEROMETER_FEET_BALL_DIAMETER_MM
#  define SPHEROMETER_FEET_BALL_DIAMETER_MM 12.7
#endif
// Radius of the bearing balls we're standing on.
static constexpr float ball_r_mm = SPHEROMETER_FEET_BALL_DIAMETER_MM / 2;

// Pins the dial indicator is connected to.
static constexpr uint8_t CLK_BIT  = (1<<3);
static constexpr uint8_t DATA_BIT = (1<<4);

static constexpr uint8_t BUTTON_BIT = (1<<1);  // A button as UI input.

// ------------------------------ nothing to be changed below --------
// ... derived from the above; let's compile-time calculate them.
static constexpr ErrorFloat d_inch = d_mm / 25.4f;
static constexpr ErrorFloat ball_r_inch = ball_r_mm / 25.4f;
static constexpr ErrorFloat d_mm_squared = d_mm * d_mm;
static constexpr ErrorFloat d_inch_squared = d_inch * d_inch;

// Data coming back from the dial indicator. The dial indicator knows
// its internal format and returns values as Millimeter or Inch.
// This is filled by the dial-indicator-specific implementation of
// ReadDialIndicator()
struct DialData {
  DialData() : value(0), negative(false), is_imperial(false), raw_count(0) {}
  ErrorFloat value;   // Absolute value in mm or inches.
  bool negative;      // true if the value is negative.
  bool is_imperial;   // Reading is in imperial units
  int32_t raw_count;  // the internal raw value.
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

static ErrorFloat calc_r(DialData dial, bool tool_referenced) {
  ErrorFloat sag = dial.value;
  // For tool-referenced mode, we have to deal with roughly half
  // the sag being contributed by the concave shape of the mirror and
  // half by the convex shape of the tool. It is not exactly half, as
  // the spherometer stands on balls, and positive ball correction is
  // needed for the concave mirror and negative ball correction is for
  // the convex tool.
  //
  // So if we calculate it correctly, the resulting formula would be
  // 'slightly' longer than the standard spherometer formula
  // (b being the ball radius, d leg radius):
  // R = b+(((sqrt(-s^6-(-12*d^2-32*b^2)*s^4-(48*d^4-320*b^2*d^2+256*b^4)*s^2+64*d^6-64*b^2*d^4)/(8*3^(3/2))+((3*(s*d^2))/2-((4*b-3*s)*(4*s*b-2*d^2-s^2))/4)/6-(4*b-3*s)^3/216)^(1/3)-(-(4*s*b-2*d^2-s^2)/6-(4*b-3*s)^2/36)/(sqrt(-s^6-(-12*d^2-32*b^2)*s^4-(48*d^4-320*b^2*d^2+256*b^4)*s^2+64*d^6-64*b^2*d^4)/(8*3^(3/2))+((3*(s*d^2))/2-((4*b-3*s)*(4*s*b-2*d^2-s^2))/4)/6-(4*b-3*s)^3/216)^(1/3)-(4*b-3*s)/6)^(2)+d^2)/(2*((sqrt(-s^6-(-12*d^2-32*b^2)*s^4-(48*d^4-320*b^2*d^2+256*b^4)*s^2+64*d^6-64*b^2*d^4)/(8*3^(3/2))+((3*(s*d^2))/2-((4*b-3*s)*(4*s*b-2*d^2-s^2))/4)/6-(4*b-3*s)^3/216)^(1/3)-(-(4*s*b-2*d^2-s^2)/6-(4*b-3*s)^2/36)/(sqrt(-s^6-(-12*d^2-32*b^2)*s^4-(48*d^4-320*b^2*d^2+256*b^4)*s^2+64*d^6-64*b^2*d^4)/(8*3^(3/2))+((3*(s*d^2))/2-((4*b-3*s)*(4*s*b-2*d^2-s^2))/4)/6-(4*b-3*s)^3/216)^(1/3)-(4*b-3*s)/6))
  // .. however, this mostly boils down to evening out
  // positive and negative of the ball leg sizes, the rest
  // of the terms contribute little.
  // So, the tool referenced mode is very closely approximated by
  // simply calculating the spherometer without ball correction and
  // we're accurate well within very small margins (better than 0.1mm for
  // the radius).

  if (tool_referenced) sag = sag / 2;
  ErrorFloat result = dial.is_imperial
    ? ((d_inch_squared + sag*sag) / (2*sag))
    : ((d_mm_squared + sag*sag) / (2*sag));
  if (!tool_referenced) {
    // In tool referenced mode, ball diameter evens out, so nothing to
    // add.
    result = result + (dial.is_imperial ? ball_r_inch : ball_r_mm);
  }
  return result;
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

  // While at sleep, we don't want button presses to use power. Also, this
  // is a nice way to measure if we are in sleep mode.
  void SleepMode(bool s) {
    if (s) {
      PORTB &= ~BUTTON_BIT;
    } else {
      PORTB |= BUTTON_BIT;
    }
  }
  bool clicked() {
    const bool current_press = (PINB & BUTTON_BIT) == 0;  // negative logic
    const bool before = previous_pressed_;
    previous_pressed_ = current_press;
    return (current_press && !before);
  }

private:
  bool previous_pressed_ = false;
};

void ShowRadiusPage(SH1106Display *disp, DialData dial, uint8_t dia_choice,
                    bool tool_referenced) {
  const ErrorFloat error_radius = calc_r(dial, tool_referenced);
  const float radius = error_radius.nominal;
  // Calculating the sag values to radius in their respective units.
  // We round the returned value to an integer, which is the type
  // we can properly string format below.
  // For imperial: fixpoint shift to display 1/10" unit
  const int32_t radius_digits = roundf(dial.is_imperial ? 10 * radius : radius);
  const bool is_overflow = (radius_digits > 9999);   // Limit digits on screen
  const int32_t display_radius = is_overflow ? 9999 : radius_digits;

  uint8_t x;
  // -- Print focal length and ƒ/-Number
  if (is_overflow) {
    // Don't show any numbers in that case. Not every useful.
    for (int y = 0; y < 32; y += 8)
      disp->FillStripeRange(0, 127, y, 0x00);
    disp->Print(font_bignumber, 46, 0, "⚠");
  }
  else {
    const float f = radius / 2;  // Focal length of a sphere.
    const float f_err = (error_radius.high - radius)/2;
    // -- ƒ/-Number according to user choice from button.
    // f/5.43 ≈ ⌀ 6"
    const float f_mm = dial.is_imperial ? 25.4 * f : f;
    const float dia = aperture_items[dia_choice].mm;
    const int32_t f_N = roundf(100 * f_mm / dia);  // 100* for 2 decimal digits
    x = disp->Print(font_smalltext, 0, 0, "ƒ/");
    x = disp->Print(font_smalltext, x, 0, strfmt(f_N, 2));
    x = disp->Print(font_smalltext, x, 0, "  ");
    x = disp->Print(font_smalltext, 64, 0, "≈");
    // Ideally, we'd like to show this inverse to better draw the attention
    // to this number and the arrow-button right next to it.
    // However, it seems to suck brightness out of that line which makes
    // it visually non-pleasing. TODO: Experiment with it once final
    // case with button is there.
    x = disp->Print(font_smalltext, x, 0, aperture_items[dia_choice].text,
                    false);

    // Print focal length in chosen units.
    constexpr int line2 = 16;
    x = disp->Print(font_smalltext, 0, line2, "ƒ=");
    const int32_t display_f = roundf(dial.is_imperial ? 10*f : f);
    x = disp->Print(font_smalltext, x, line2,
                    strfmt(display_f, dial.is_imperial ? 1:0, 5));
    const int32_t display_error_f = roundf(dial.is_imperial ? 10*f_err : f_err);
    x = disp->Print(font_smalltext, x, line2, "±"); // show error as plusminus
    x = disp->Print(font_smalltext, x, line2,
                    strfmt(display_error_f, dial.is_imperial ? 1:0, 0));
    x = disp->Print(font_smalltext, x, line2, dial.is_imperial ? "\"  " : "mm");
    // since we're hugging the ±-value right after the value we show, we need
    // to make sure to clean everything after the value we've written to not
    // leave leftover 'mm' or '"' suffix behind when numbers get shorter.
    // Our text is two stripes high, so let's fill these.
    disp->FillStripeRange(x, 127, line2, 0x00);
    disp->FillStripeRange(x, 127, line2+8, 0x00);
  }

  // -- Printing the radius in a large font.

  // Make sure that it is clear we're talking about the sphere radius
  disp->Print(font_smalltext, 0, 32, "r=");

  // If the value is too large, we don't want to overflow the display.
  // Instead, we clamp it to highest value and show a little > indicator.
  if (is_overflow) {
    disp->Print(font_smalltext, 0, 48, ">");  // 'overflow' indicator
  } else {
#ifndef DISABLE_TOOL_REFERENCE_FEATURE
    disp->Print(font_smalltext, 0, 48, tool_referenced ? "⯊" : "▂");
#else
    disp->Print(font_smalltext, 0, 48, " ");
#endif
  }

  // Different formatting of numbers in different units, including suffix
  if (dial.is_imperial) {
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

  DialData last_dial;
  uint8_t last_aperture_choice = 0xff;  // outside range, so guaranteed refresh

  uint8_t off_cycles = 0;
  uint8_t aperture_choice = 0;
  bool tool_referenced = false;

  constexpr uint8_t kPowerOffAfterCycles = 50;
  constexpr uint8_t kStartShowingOutro = 4;
  DialData dial;
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
      last_aperture_choice = 0xff;  // Force refresh of screen after we get out
      disp.Print(font_smalltext, 0, 0, "©Henner Zeller");
      disp.Print(font_tinytext, 0, 16, "github hzeller/");
      disp.Print(font_tinytext, 0, 32, "digi-spherometer");
      disp.Print(font_tinytext, 0, 48, "    GNU GPL");
    }
    else if (off_cycles > kPowerOffAfterCycles) {
      disp.SetOn(false);
      button.SleepMode(true);
      SleepTillDialIndicatorClocksAgain();  // ZZzzz...

      // ... We're here after wakeup
      button.SleepMode(false);
      disp.Reset();      // Might've slept a long time. Make sure display ok.
      tool_referenced = false;
    }

    if (off_cycles)
      continue;

    if (last_aperture_choice == 0xff
        || last_dial.negative != dial.negative
        || last_dial.is_imperial != dial.is_imperial
        || is_flat(last_dial.raw_count) != is_flat(dial.raw_count)) {
      disp.ClearScreen();  // Visuals will change. Clean-slatify.
    }

    if (is_flat(dial.raw_count)) {
#ifdef DISABLE_TOOL_REFERENCE_FEATURE
      disp.Print(font_smalltext, 48, 0, "flat");
#else
      if (button.clicked()) tool_referenced = !tool_referenced;
      disp.Print(font_smalltext, 60, 0,  " ▂ flat", !tool_referenced);
      disp.Print(font_smalltext, 60, 16, " ⯊ tool", tool_referenced);
#endif
      disp.Print(font_bignumber, 12, 32, "ZERO");
    }
    else if (!dial.negative) {
      disp.Print(font_bignumber, 46, 0, "⚠");
      disp.Print(font_smalltext, 0, 32, "Please zero on");
#ifdef DISABLE_TOOL_REFERENCE_FEATURE
      disp.Print(font_smalltext, 8, 48, "flat surface");
#else
      disp.Print(font_smalltext, 8, 48, "ref. surface");
#endif
    }
    else {
      if (button.clicked()) {
        aperture_choice += 1;
        if (aperture_choice >= kApertureChoices) aperture_choice = 0;
      }
      if (dial.raw_count != last_dial.raw_count
          || dial.is_imperial != last_dial.is_imperial
          || last_aperture_choice != aperture_choice) {
        ShowRadiusPage(&disp, dial, aperture_choice, tool_referenced);
      }
    }

    last_dial = dial;
    last_aperture_choice = aperture_choice;
  }
}
