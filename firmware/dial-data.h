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
#ifndef DIAL_DATA_H
#define DIAL_DATA_H

#include <stdint.h>
#include "error-float.h"

// Data coming back from the dial indicator.
// The dial indicator knows
// its internal format and returns values as Millimeter or Inch.
// This is filled by the dial-indicator-specific implementation of
// ReadDialIndicator() (in dial-indicator-*.h)
struct DialData {
  DialData() : value(0), negative(false), is_imperial(false), raw_count(0) {}
  ErrorFloat value;   // Absolute value in mm or inches.
  bool negative;      // true if the value is negative.
  bool is_imperial;   // Reading is in imperial units
  int32_t raw_count;  // the internal raw value.

  bool operator == (const DialData other) const {
    return raw_count == other.raw_count && negative == other.negative
      && is_imperial == other.is_imperial;
  }
  bool operator != (const DialData other) const {
    return !(*this == other);
  }
};
#endif  // DIAL_DATA_H
