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
#ifndef SPHEROMETER_CALCULATION_H
#define SPHEROMETER_CALCULATION_H

#include "error-float.h"
#include "dial-data.h"

/*
 * in a separate file so that we can use it in the firmware as well as
 * compiling it on the host for the host-testing binary.
 */

namespace spherometer {
// Distance center to feet. Radius of the Spherometer-feet circle.
#ifndef SPHEROMETER_RADIUS_MM
#  define SPHEROMETER_RADIUS_MM 50
#endif
#ifndef SPHEROMETER_RADIUS_ERROR_MM
#  define SPHEROMETER_RADIUS_ERROR_MM 0.1
#endif
static constexpr ErrorFloat leg_d_mm(SPHEROMETER_RADIUS_MM,
                                     SPHEROMETER_RADIUS_ERROR_MM);

#ifndef SPHEROMETER_FEET_BALL_DIAMETER_MM
#  define SPHEROMETER_FEET_BALL_DIAMETER_MM 12.7
#endif
// Radius of the bearing balls we're standing on.
static constexpr float ball_r_mm = SPHEROMETER_FEET_BALL_DIAMETER_MM / 2;

// ------------------------------ nothing to be changed below --------
// ... derived from the above; let's compile-time calculate them.
static constexpr ErrorFloat leg_d_inch = leg_d_mm / 25.4f;
static constexpr ErrorFloat ball_r_inch = ball_r_mm / 25.4f;
static constexpr ErrorFloat leg_d_mm_squared = leg_d_mm * leg_d_mm;
static constexpr ErrorFloat leg_d_inch_squared = leg_d_inch * leg_d_inch;

// Returns the absolute value of the sphere radius including error
// calculations.
// Given: the dial reading in "dial" and if the value is tool referenced
// (if configured).
inline ErrorFloat calc_r(DialData dial, bool
#if TOOL_REFERENCE_FEATURE
                         tool_referenced
#endif
                         ){
  ErrorFloat sag = dial.value;

#if TOOL_REFERENCE_FEATURE
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
#endif

  // Minimize calculations: use compile-time precalc half squared leg distance.
  const ErrorFloat saghalf = sag / 2;
  const ErrorFloat result = (dial.is_imperial
                             ? saghalf + (leg_d_inch_squared/2) / sag
                             : saghalf + (leg_d_mm_squared/2) / sag);

#if TOOL_REFERENCE_FEATURE
  if (tool_referenced)
    return result;   // No correction needed (see above)
#endif

  // Correct depending on negative sag (=concave) or positive sag (=convex)
  const ErrorFloat ball_correct = (dial.is_imperial ? ball_r_inch : ball_r_mm);
  return dial.negative ? result + ball_correct : result - ball_correct;
}
}  // namespace calc_sphere
#endif  // SPHEROMETER_CALCULATION_H
