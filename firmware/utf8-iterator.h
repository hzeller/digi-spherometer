// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// Copyright (C) 2013 Henner Zeller <h.zeller@acm.org>
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
#ifndef RPI_GRAPHICS_UTF8_H
#define RPI_GRAPHICS_UTF8_H

#include <stdint.h>

// Utility function that reads UTF-8 encoded codepoints from byte iterator.
// No error checking, we assume string is UTF-8 clean.
template <typename byte_iterator>
uint16_t utf8_next_codepoint(byte_iterator &it) {
  uint16_t cp = *it;
  ++it;
  if (cp < 0x80) {
    return cp;   // iterator already incremented.
  }
  else if ((cp & 0xE0) == 0xC0) {
    cp = ((cp & 0x1F) << 6) + (*it & 0x3F);
  }
  else if ((cp & 0xF0) == 0xE0) {
    cp = ((cp & 0x0F) << 12) + ((*it & 0x3F) << 6);
    cp += (*++it & 0x3F);
  }
  // We don't deal with longer sequences here.
  ++it;
  return cp;
}

#endif  // RPI_GRAPHICS_UTF8_H
