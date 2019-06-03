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
#ifndef ERROR_FLOAT_H
#define ERROR_FLOAT_H

// Super-simple float values that propagate errors.
// Note, these are not generalized and only work for our limited purpose,
// essentially only dealing with positive numbers.
struct ErrorFloat {
  constexpr ErrorFloat(float min, float value, float max)
    : low(min), nominal(value), high(max) {}
  constexpr ErrorFloat(float value, float e)
    : ErrorFloat(value-e, value, value+e){}
  constexpr ErrorFloat(float value) : ErrorFloat(value, value, value){}

  // To avoid problems with small values get out of range or non-divisible
  // due to applying error margins, this forces them to behave.
  inline void force_positive() { if (low <= 0.0f) low = nominal; }

  float low;
  float nominal;
  float high;
};
constexpr inline ErrorFloat operator*(ErrorFloat a, ErrorFloat b) {
  return ErrorFloat(a.low*b.low, a.nominal*b.nominal, a.high*b.high);
}
constexpr inline ErrorFloat operator+(ErrorFloat a, ErrorFloat b) {
  return ErrorFloat(a.low+b.low, a.nominal+b.nominal, a.high+b.high);
}
constexpr inline ErrorFloat operator/(ErrorFloat a, ErrorFloat b) {
  return ErrorFloat(a.low/b.high, a.nominal/b.nominal, a.high/b.low);
}

#endif // ERROR_FLOAT_H
