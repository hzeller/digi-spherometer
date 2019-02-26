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
#ifndef AUTOLET_DIAL_INDICATOR_H
#define AUTOLET_DIAL_INDICATOR_H

// This is a header file that can be implemted differently depending on
// the type of dial indicator (there might be different ways to read out
// the data.
// Then in main.cc, we include the right now.
//
// All we requires is to have some source-code compatibility (e.g. the layout
// of DialData can be different.
//
// (a) a DialDat data type that allows to access value, negative and is_imperial
// (b) a function called ReadDialIndicator()


struct DialData {
  int32_t value : 20;      // 20 bits of raw count data
  uint8_t negative : 1;    // if true, value avoe is a negative number.
  uint8_t off : 1;         // dial indicator off, couldn't read data.
  uint8_t dummy_notused : 1;
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
DialData ReadDialIndicator(uint8_t clk_bit, uint8_t data_bit) {
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
    if (PINB & clk_bit) {
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
    while ((PINB & clk_bit) == 0)  // Wait for clk to go high.
      ;
    while (PINB & clk_bit) {      // Wait for negative edge
      clock_inactive_count++;
      if (clock_inactive_count > kOffDetectionCount) {
        result.data.off = 1;
        return result.data;
      }
    }
    if ((PINB & data_bit) == 0)
      result.bits |= current_bit;
    current_bit <<= 1;
  }
  return result.data;
}

#endif // AUTOLET_DIAL_INDICATOR_H
