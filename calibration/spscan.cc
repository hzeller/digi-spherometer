/*
 * (c) 2019 H.Zeller@acm.org CC-BY-SA License
 *
 * Given 4 space-separated x/y coordinates, calculate the
 * radius and center of the circle defined by the first three coordinates.
 *
 * Then print how far the last coordinate (in our case where the dial indicator
 * is) is away from the edge of that circle.
 *
 * The coordinates are read from stdin; these coordinates are typically raw
 * coordinates you got from a high-resolution scan.
 * there is a single commandline paraemter that is interpreted as DPI to
 * convert the value back to mm.
 *
 * Example:
 * Say this is our file with  four coordinates
 cat positions.txt
 6385  9605
 14629 9845
 10731 2616
 10590 7313

 Then if we run the binary, we get the relevant information back. Here, the
 coordinates were read from a 2400dpi scan:

 ./spscan 2400 < ~/positions.txt
 Circle radius...............: 50.289mm
 Indicator to true center....:  0.573mm
 Effective Spherometer radius: 49.716mm

 Our spherometer radius with these measurements is 49.716mm
 */
#include <stdio.h>
#include <math.h>
#include <iostream>

static const double kDefaultDPI = 2400.0;
static double dpi = kDefaultDPI;

static double to_mm(double x) { return 25.4 * x / dpi; }
static double sq(double x) { return x*x; }

int main(int argc, char *argv[]) {
    if (argc == 2) dpi = atof(argv[1]);

    // First 6 values: measured x/y coordinates of balls.
    double x1, y1, x2, y2, x3, y3;
    std::cin >> x1 >> y1 >> x2 >> y2 >> x3 >> y3;

    // Last 2 values: x/y of where the dial indicator center measures.
    double dial_x, dial_y;
    std::cin >> dial_x >> dial_y;

    // Determine the circle center and radius given the three points.
    // Given circle equation A(x²+y²) + Bx + Cy + D = 0, we find A, B, C, D
    // that satisfies the equation for the three x/y pairs we have.
    const double sqsum1 = sq(x1) + sq(y1);
    const double sqsum2 = sq(x2) + sq(y2);
    const double sqsum3 = sq(x3) + sq(y3);

    const double A = x1*(y2 - y3) - y1*(x2 - x3) + x2*y3 - x3*y2;
    const double B = sqsum1*(y3 - y2) + sqsum2*(y1 - y3) + sqsum3*(y2 - y1);
    const double C = sqsum1*(x2 - x3) + sqsum2*(x3 - x1) + sqsum3*(x1 - x2);
    const double D = (sqsum1*(x3*y2 - x2*y3) +
                      sqsum2*(x1*y3 - x3*y1) +
                      sqsum3*(x2*y1 - x1*y2));
    const double radius = sqrt((B*B + C*C - 4*A*D) / (4*A*A));
    const double c_x = -B/(2*A);  // center x/y
    const double c_y = -C/(2*A);
    const double center_distance = hypot(c_x - dial_x, c_y - dial_y);

    printf("Circle radius...............: %6.3fmm\n", to_mm(radius));
    printf("Indicator to true center....: %6.3fmm\n", to_mm(center_distance));
    printf("Effective Spherometer radius: %6.3fmm\n",
           to_mm(radius - center_distance));
}
