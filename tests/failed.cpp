
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <float.h>

#include "../src/latest-musl-libm/libm.h"

_Noreturn void abort(void);

#define STRINGIFY0(x) #x
#define STRINGIFY(x)  STRINGIFY0(x)

#define ASSERTF(cond, ...) do if (!(cond)) {         \
  fputs(__FILE__":"STRINGIFY(__LINE__)": ", stderr); \
  fputs(__func__, stderr);                           \
  fputs(": Assertion `"#cond"' failed: ", stderr);   \
  fprintf(stderr, __VA_ARGS__);                      \
  abort();                                           \
} while (0)

#define verify(cond, x) ASSERTF(cond, "at %a\n", (double)(x))

double reinterpret(uint64_t x) {
  return reinterpret_cast<double&>(x);
}

uint64_t reinterpret(double x) {
  return reinterpret_cast<uint64_t&>(x);
}

static inline bool faithful(double x, double y) {
  const uint64_t mask = 0x1FFFFFFFULL;
  uint64_t a = reinterpret(x);
  uint64_t b = reinterpret(y);
  // return a - b + mask <= 2 * mask;
  return a - b <= mask;
}

static void test(double f(double), long double g(long double)) {
  const uint64_t step = 0x000000088D16E29BULL;

  assert(f(INFINITY) == INFINITY);
  assert(f(-0.0) == -INFINITY);
  assert(isnan(f(-INFINITY)));

  for (uint64_t i = 0ULL; i < 0x7FF0000000000000ULL; i += step) {
    double x = reinterpret(i);
    verify(faithful(f(x), g(x)), x);
  }

  for (uint64_t i = 0x7FF8000000000000ULL; i < 0x8000000000000000ULL; i += step) {
    double x = reinterpret(i);
    verify(isnan(f(+x)), x);
    verify(isnan(f(-x)), x);
  }

  for (uint64_t i = 0x8000000000000001ULL; i < 0xFFF0000000000000ULL; i += step) {
    double x = reinterpret(i);
    verify(isnan(f(x)), x);
  }
}

void computeLine(uint32_t y) {
  static int i = 0;

  double width  = 3328;
  double height = 1634;
  double translateX = 3328 * (1.0 / 1.6);
  double translateY = 1634 * (1.0 / 2.0);
  double scale      = 10.0 / 1634.0;
  double imaginary  = (y - translateY) * scale;
  double realOffset = translateX * scale;
  uint32_t stride   = y * 3328;
  double invLimit   = 1.0 / 40.0;

  uint32_t minIterations = 8;

  for (uint32_t x = 0; x < 4000; ++x) {
    double real = x * scale - realOffset;

    // Iterate until either the escape radius or iteration limit is exceeded
    double ix = 0.0, iy = 0.0, ixSq = 0.0, iySq = 0.0;
    uint32_t iteration = 0;
    while ((ixSq = ix * ix) + (iySq = iy * iy) <= 4.0) {
      iy = 2.0 * ix * iy + imaginary;
      ix = ixSq - iySq + real;
      if (iteration >= 40) break;
      iteration++;
    }

    // Do a few extra iterations for quick escapes to reduce error margin
    while (iteration < minIterations) {
      double ixNew = ix * ix - iy * iy + real;
      iy = 2.0 * ix * iy + imaginary;
      ix = ixNew;
      iteration++;
    }

    // Iteration count is a discrete value in the range [0, limit] here, but we'd like it to be
    // normalized in the range [0, 2047] so it maps to the gradient computed in JS.
    // see also: http://linas.org/art-gallery/escape/escape.html
    uint32_t col = 2048 - 1;
    double sqd = ix * ix + iy * iy;
    if (sqd > 1.0) {
      // double actual = _log2(0.5 * _log(sqd));
      // double expected = log2(0.5 * log(sqd));
      double actual = _log(sqd);
      double expected = log(sqd);
      if (!faithful(actual, expected)) {
        printf("% 4d) arg: %.17e, actual: %.17f, expected: %.17f\n", i, sqd, actual, expected);
        i++;
      }
    }
  }
}




int main() {
  // test musl 1.2.0 log vs libc logl
  // fputs("> Tests starts:\n", stdout);
  // fputs("1) log(x)...", stdout);
  // fflush(stdout);
  // test(_log, logl);
  // fputs("passed\n", stdout); fflush(stdout);
  // fputs("> Tests done without any issues!\n", stdout);

  for (int y = 1000; y < 4000; y++) {
    computeLine(y);
  }
}