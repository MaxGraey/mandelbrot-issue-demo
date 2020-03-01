
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <float.h>

#include "../src/latest-musl-libm/libm.h"

#define ABS(x)  ((signed)(x) < 0 ? -(x) : (x))

int64_t ulpDiffs(double x, double y) {
  if (isnan(x) && isnan(y)) return  0;
  if (isnan(x) || isnan(y)) return -1;

  int64_t lc = reinterpret_cast<int64_t&>(x);
  int64_t rc = reinterpret_cast<int64_t&>(y);

  if ((lc < 0) != (rc < 0)) {
    return x == y ? 0 : -1;
  }

  return ABS(lc - rc);
}

int main() {
  const double inputs[] = {
    9.39272367658092655e+00,
    7.14726094603619089e+00,
    8.60221196398579480e+00,
    4.13931110013671955e+00,
    7.43155422527207676e+00,
    8.17974828584226366e+00,
    4.94454417465960727e+00,
    6.70535708800538277e+00,
    7.98827495796161813e+00,
    5.95566127061565265e+00,
    1.07102800210768265e+01,
    8.57346179090154159e+00
  };

  printf("log2(0.5 * log(x)):\n");

  for (int i = 0; i < sizeof inputs / 8; i++) {
    double input    = inputs[i];
    double actual   = _log2(0.5 * _log(input)); // musl 1.2.0
    double expected =  log2(0.5 *  log(input)); // libc
    printf("\narg:    %.17g\n", input);
    printf("---------------------------------------------------\n");
    printf("actual:% .17f (% .13a)\n", actual,   actual);
    printf("expect:% .17f (% .13a)\n", expected, expected);
    printf("ULP diff: %llu\n", ulpDiffs(actual, expected));
  }
  return 0;
}