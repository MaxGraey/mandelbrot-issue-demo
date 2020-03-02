// see: https://en.wikipedia.org/wiki/Mandelbrot_set

#include <stdint.h>
#include <emscripten/emscripten.h>

#include "latest-musl-libm/libm.h"

#define NUM_COLORS 2048

#define MIN(x,y)   ((x) < (y) ? (x) : (y))
#define MAX(x,y)   ((x) > (y) ? (x) : (y))
#define ABS(x)     ((signed)(x) < 0 ? -(x) : (x))

extern "C" {
  extern double js_log_f64(double x);
  extern double js_log2_f64(double x);
}

uint16_t* image = (uint16_t*)0;

/** Clamps a value between the given minimum and maximum. */
inline static double clamp(double value, double minValue, double maxValue) {
  return MIN(MAX(value, minValue), maxValue);
}

/** Computes a single line in the rectangle `width` x `height`. */
void EMSCRIPTEN_KEEPALIVE computeLine(uint32_t y, uint32_t width, uint32_t height, uint32_t limit) {
  double translateX = width  * (1.0 / 1.6);
  double translateY = height * (1.0 / 2.0);
  double scale      = 10.0 / MIN(3 * width, 4 * height);
  double imaginary  = (y - translateY) * scale;
  double realOffset = translateX * scale;
  uint32_t stride   = y * width;
  double invLimit   = 1.0 / (double)limit;

  uint32_t minIterations = MIN(8, limit);

  for (uint32_t x = 0; x < width; ++x) {
    double real = x * scale - realOffset;

    // Iterate until either the escape radius or iteration limit is exceeded
    double ix = 0.0, iy = 0.0, ixSq = 0.0, iySq = 0.0;
    uint32_t iteration = 0;
    while ((ixSq = ix * ix) + (iySq = iy * iy) <= 4.0) {
      iy = 2.0 * ix * iy + imaginary;
      ix = ixSq - iySq + real;
      if (iteration >= limit) break;
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
    uint32_t col = NUM_COLORS - 1;
    double sqd = ix * ix + iy * iy;
    if (sqd > 1.0) {
      // NOTE: _log2 /_log imported from latest-musl-libm and contain precision bug
      double frac = _log2(0.5 * _log(sqd));
      col = uint32_t((NUM_COLORS - 1) * clamp((iteration + 1 - frac) * invLimit, 0.0, 1.0));
    }
    image[stride + x] = col;
  }
}

int64_t ulpDiff(double x, double y) {
  if ((x != x) && (y != y)) return  0;
  if ((x != x) || (y != y)) return -1;

  int64_t lc = reinterpret_cast<int64_t&>(x);
  int64_t rc = reinterpret_cast<int64_t&>(y);

  if ((lc < 0) != (rc < 0)) {
    return x == y ? 0 : -1;
  }

  return ABS(lc - rc);
}

double EMSCRIPTEN_KEEPALIVE computeLog2LogSqrt(double x) {
  return _log2(0.5 * _log(x));
}

double EMSCRIPTEN_KEEPALIVE computeULP(double x) {
  double actual    = _log2(0.5 * _log(x));
  double excpected = js_log2_f64(0.5 * js_log_f64(x));
  return double(ulpDiff(actual, excpected));
}
