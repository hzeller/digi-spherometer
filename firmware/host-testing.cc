// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// Copyright (C) 2023 Henner Zeller <h.zeller@acm.org>
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

/*
 * Little program to test calculations manually on the development
 * machine.
 */

#include "spherometer-calculation.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The measurement error from the datahseet of the dial indicator.
static constexpr float kSimulatedDialErrorMM = 0.003;

// Pretend to be a dial indicator, just reading the value from the string
// and emitting DialData as it was coming from the indicator.
static DialData ParseDialData(const char *val) {
  DialData result{};
  result.is_imperial = false;

  char *end = nullptr;
  const float parsed = strtof(val, &end);
  if (end) {
    if (strcmp(end, "in") == 0)
      result.is_imperial = true;
    else if (*end == '\0' || strcmp(end, "mm") == 0)
      result.is_imperial = false;
    else {
      fprintf(stderr, "unexpected suffix on '%s'. Allowed: 'mm' or 'in'\n",
              val);
    }
  }

  result.negative = (parsed < 0);  // Dial indicator reports sign separately

  // Simulate error coming from dial indicator.
  if (result.is_imperial) {
    result.value = { fabs(parsed), kSimulatedDialErrorMM / 25.4 };
  } else {
    result.value = { fabs(parsed), kSimulatedDialErrorMM };
  }
  result.value.force_positive();  // Clamp error margings to not go negative.

  return result;
}

void PrintErrorFloat(const ErrorFloat &f, const char *unit, const char *m) {
  printf("%8.3f%s", f.nominal, unit);
  const bool has_plus = (f.high > f.nominal);
  const bool has_minus = (f.low < f.nominal);
  int accuracy = 3;
  if (f.high - f.nominal < 1e-3) accuracy = 4;
  if (f.nominal > 100) accuracy = 2;
  if (has_plus || has_minus) printf(" (");
  if (has_minus) printf("-%.*f%s", accuracy, f.nominal - f.low, unit);
  if (has_plus) {
    printf("%s+%.*f%s", has_minus ? "/" : "",
           accuracy, f.high - f.nominal, unit);
  }
  if (has_plus || has_minus) printf(")");
  printf(" %s", m);
}

static int usage(const char *progname) {
  fprintf(stderr, "%s <value> [<value>...]\n", progname);
  fprintf(stderr, "Values are something like 0.72mm or 0.03in.\n"
          "Without suffix, they are interpreted as mm\n");
  return 1;
}

int main(int argc, char *argv[]) {
  printf("== Spherometer constants ==\n");
  printf("Ball radius       : %8.3fmm\n", spherometer::ball_r_mm);
  printf("Leg radius        : ");
  PrintErrorFloat(spherometer::leg_d_mm, "mm", "\n");
  printf("Assumed Dial error: %8.3fmm\n", kSimulatedDialErrorMM);
  printf("Assuming concave mirrors result in negative value readings.\n\n");

  if (argc <= 1) return usage(argv[0]);
  printf("== Measurements ==\n");
  for (int i = 1; i < argc; ++i) {
    const DialData dial_value = ParseDialData(argv[i]);
    const ErrorFloat radius = spherometer::calc_r(dial_value, false);
    const char *unit = dial_value.is_imperial ? "in" : "mm";
    printf("%s", dial_value.negative ? "-" : " ");
    PrintErrorFloat(dial_value.value, unit, " => ");
    PrintErrorFloat(radius, unit,
                    dial_value.negative ? "concave radius\n"
                                        : "convex radius\n");
  }
}
