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

#include "strfmt.h"

const char *strfmt(int32_t v, int8_t decimals, int8_t total_len) {
  static char scratch[16];
  constexpr auto buflen = sizeof(scratch);
  char *buffer = scratch;
  const bool is_neg = (v < 0);
  if (is_neg) v = -v;
  buffer = buffer + buflen - 1;
  *buffer-- = '\0';
  const char *pad_to = (total_len > 0) ? (buffer - total_len) : buffer;
  bool is_first = true;  // properly print and decimal-handling zero value.
  while (is_first || v > 0) {
    *buffer-- = (v % 10) + '0';
    v /= 10;
    if (--decimals == 0)
      *buffer-- = '.';
    is_first = false;
  }
  if (decimals > 0) {
    while (decimals-- > 0)
      *buffer-- = '0';
    *buffer-- = '.';
  }
  if (is_neg) *buffer-- = '-';
  while (pad_to < buffer)
    *buffer-- = ' ';
  return buffer + 1;
}
