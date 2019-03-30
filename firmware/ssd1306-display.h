// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// Copyright (C) 2018 Henner Zeller <h.zeller@acm.org>
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

#include <stdint.h>

#include "tiny-i2c-master.h"
#include "bdfont-support.h"

// A super basic interface to the SSD1306 display that doesn't have any
// frame-buffer (as the whole RAM of the attiny is just a fraction of what
// would be needed for a full display of 64x128).
//
// Instead, we allow provide methods to directly dump out bytes stored in
// progmem.
// We use this to allow writing text to the display with compiled-in fonts.
class SSD1306Display {
public:
  SSD1306Display();

  // Re-initialize.
  void Reset();

  void ClearScreen();

  // Switch display light on/off
  void SetOn(bool on);

  // Write a string with the given font that is stored in progmem memory.
  //
  // Since there is not enough RAM space in tha Attiny, we directly read
  // glyphs from progmem.
  // While the pointer is only to a MetaFont, is assumed that
  // the actual font data is layed out directly after the MetaFont data
  // in memory as is done with the compile-time struct-init of the font.
  //
  // "xpos" is the x coordinate, "ypos" the y-coordiante (but note that only
  // multiple of 8 are supported currently).
  // "utf8_text" is a UTF-8 string.
  //
  // Returns the new x-position at the end of the string.
  //
  // Characters that are not present in the font are skipped.
  // (Fonts are allowed to only contain characters that are needed in the
  //  application, so they can sparsely populated. The glyphs are expected to
  //  be sorted by codepoint)
  uint8_t Print(const FontData &font,
                uint8_t xpos, uint8_t ypos, const char *utf8_text);

  // Fills a stripe in x-range [x_from...x_to). Stripe is given by y coordinate
  // ypos, but note that only mulitple of 8 are handled.
  // fill_mask provides the pattern.
  void FillStripeRange(uint8_t x_from, uint8_t x_to, uint8_t ypos,
                       uint8_t fill_mask);
};
