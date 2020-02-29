// see: https://en.wikipedia.org/wiki/Mandelbrot_set

#include <stdint.h>
#include <emscripten/emscripten.h>

#include "latest-musl-libm/libm.h"

#define NUM_COLORS 2048

#define MIN(x,y)   ((x) < (y) ? (x) : (y))
#define MAX(x,y)   ((x) > (y) ? (x) : (y))

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
