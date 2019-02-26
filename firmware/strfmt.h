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
#ifndef STRFMT_H
#define STRFMT_H

#include <stdint.h>

// Utility function to print fixed point numbers.
// Format number "number". If "decimals" is given, a decimal point
// is placed in front of these many digits from the right.
// The string is padded with spaces at the front to satisfy "total_len".
//
// Returns pointer to NUL-terminated, statically allocated string.
// (invalidated with the next strfmt() call).
const char *strfmt(int32_t number, int8_t decimals, int8_t total_len = -1);
#endif // STRFMT_H
