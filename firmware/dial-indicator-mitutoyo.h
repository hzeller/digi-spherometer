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
#ifndef DIAL_INDICATOR_MITUTOYO_H
#define DIAL_INDICATOR_MITUTOYO_H

// Read Mitutoyo dial indicator (tested with Mitutoyo 543-791B).
//
// Returns 'false' if no data arrives from the indicator.
static inline bool ReadDialIndicator(uint8_t clk_bit, uint8_t data_bit,
                                     DialData *data) {
  constexpr uint16_t kInitialWaitTime = 5000;
  constexpr uint16_t kOffDetectionCount = 65000;
  constexpr uint8_t kDataNibbles = 13;

  uint8_t nibbles[kDataNibbles];

  // Wait until we have a stable time of non-clocking, so that we don't
  // jump into the middle of some clock cycle.
  uint16_t stable_clock_off_period = kInitialWaitTime;
  uint16_t clock_inactive_count = 0;
  while (stable_clock_off_period--) {
    if (PINB & clk_bit) {
      clock_inactive_count++;
    } else {
      clock_inactive_count = 0;
      stable_clock_off_period = kInitialWaitTime;
    }
    if (clock_inactive_count > kOffDetectionCount) {
      return false;
    }
  }

  // Data is clocked out in 13 nibbles, of which there are
  //  0..3  unused
  //  4     indicator if negative.
  //  5..10 BCD encoded reading, MSD first (individual bits are sent LSB first).
  //  11    Number of significant digits (we don't use this value)
  //  12    indicator if value is in imperial units.

  for (uint8_t n = 0; n < kDataNibbles; ++n) {
    clock_inactive_count = 0;
    uint8_t nibble = 0;
    for (uint8_t bit = 1; bit != 0x10; bit <<= 1) {
      while ((PINB & clk_bit) == 0)  // Wait for clk to go high.
        ;
      while (PINB & clk_bit) {       // Wait for negative edge
        clock_inactive_count++;
        if (clock_inactive_count > kOffDetectionCount) {
          return false;
        }
      }
      if ((PINB & data_bit) != 0)
        nibble |= bit;     // Bits come least significant first over the wire
    }
    nibbles[n] = nibble;
  }

  // Convert
  data->negative = (nibbles[4] == 0b1000);
  data->abs_value = nibbles[5];
  for (int i = 6; i <= 10; ++i)  // BCD conversion.
    data->abs_value = data->abs_value * 10 + nibbles[i];
  data->is_imperial = (nibbles[12] == 0b0001);
  return true;
}

#endif // DIAL_INDICATOR_MITUTOYO_H
