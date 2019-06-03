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
#ifndef DIAL_INDICATOR_AUTOUTLET_H
#define DIAL_INDICATOR_AUTOUTLET_H

// This is for this one https://www.amazon.com/gp/product/B07C63VFN3

#ifndef INDICATOR_DECIMALS
#  define INDICATOR_DECIMALS 3
#endif

// Read the dial indicator (my model: AUTOUTLET digital indicator with 1μm res).
//
// Returns 'false' if no data arrives from the indicator.
static inline bool ReadDialIndicator(uint8_t clk_bit, uint8_t data_bit,
                                     DialData *data) {
  constexpr uint16_t kInitialWaitTime = 5000;
  constexpr uint16_t kOffDetectionCount = 65000;

  // Wait until we have a stable time of non-clocking, so that we don't
  // jump into the middle of some clock cycle.
  uint16_t stable_clock_off_period = kInitialWaitTime;
  uint16_t clock_inactive_count = 0;
  while (stable_clock_off_period--) {
    if (PINB & clk_bit) {
      clock_inactive_count++;
      stable_clock_off_period = kInitialWaitTime;
    } else {
      clock_inactive_count = 0;
    }
    if (clock_inactive_count > kOffDetectionCount) {
      return false;
    }
  }

  // Data is clocked out LSB first. Data is read on rising edge of CLK.
  // We get 24 bits, 20 of which are the read value (or number*5 for imperial,
  // as they count the last half digit) and a negate and is_imperial bit.
  //
  // Since we're level converting the inputs from 1.5V to VCC with a transistor,
  // the signals are inverted, i.e. we're looking at the _falling_ edge of
  // CLK and invert the bit we read.

  union {
    struct {
      int32_t abs_value : 20;   // 20 bits of raw count data. Absolute value.
      uint8_t negative : 1;     // if true, value above is a negative number.
      uint8_t dummy_notused : 2;
      uint8_t is_imperial : 1;  // Reading is in imperial units
    } data;
    uint32_t bits;
  } result;
  result.bits = 0;

  uint32_t current_bit = 1;
  clock_inactive_count = 0;
  for (uint8_t i = 0; i < 24; ++i) {
    while ((PINB & clk_bit) == 0)  // Wait for clk to go high.
      ;
    while (PINB & clk_bit) {       // Wait for negative edge
      clock_inactive_count++;
      if (clock_inactive_count > kOffDetectionCount) {
        return false;
      }
    }
    if ((PINB & data_bit) == 0)
      result.bits |= current_bit;
    current_bit <<= 1;
  }

  // Fill data from read value.
  data->is_imperial = result.data.is_imperial;

  // We need to convert it to the expected micrometer resolution; the
  // interpretation of the raw count depends on number of digits the devices
  // support.
  //
  // There is absolutely no documentation about the accuracy of these meters,
  // which is very suspicious of course.
  // Let's be very generous here and let's assume they are only slightly worse
  // than the Mitutoyos.
#if INDICATOR_DECIMALS == 3
  if (result.data.is_imperial) {
    data->value = { result.data.abs_value * 0.00005, 0.0002 };
  } else {
    data->value = { result.data.abs_value * 0.001, 0.005 };
  }
#elif INDICATOR_DECIMALS == 2
  if (result.data.is_imperial) {
    data->value = { result.data.abs_value * 0.0005, 0.002 };
  } else {
    data->value = { result.data.abs_value * 0.01, 0.05 };
  }
#else
#  error "Unhandled INDICATOR_DECIMALS value. Supporting 2 and 3."
#endif
  data->value.force_positive();
  data->negative = result.data.negative;
  data->raw_count = result.data.abs_value;

  return true;
}

#endif // DIAL_INDICATOR_AUTOUTLET_H
