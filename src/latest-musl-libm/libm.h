#ifndef _LIBM_H
#define _LIBM_H

#include <stdint.h>
#include <float.h>
// #include <math.h>
// #include <endian.h>
// #include "fp_arch.h"

// part from math.h

#ifdef __cplusplus
extern "C" {
#endif

#include <features.h>

#define __NEED_float_t
#define __NEED_double_t
#include <bits/alltypes.h>

#if 100*__GNUC__+__GNUC_MINOR__ >= 303
#define NAN       __builtin_nanf("")
#define INFINITY  __builtin_inff()
#else
#define NAN       (0.0f/0.0f)
#define INFINITY  1e5000f
#endif

#define HUGE_VALF INFINITY
#define HUGE_VAL  ((double)INFINITY)
#define HUGE_VALL ((long double)INFINITY)

#define MATH_ERRNO  1
#define MATH_ERREXCEPT 2
#define math_errhandling 2

#define FP_ILOGBNAN (-1-0x7fffffff)
#define FP_ILOGB0 FP_ILOGBNAN

#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

#ifdef __FP_FAST_FMA
#define FP_FAST_FMA 1
#endif

#ifdef __FP_FAST_FMAF
#define FP_FAST_FMAF 1
#endif

#ifdef __FP_FAST_FMAL
#define FP_FAST_FMAL 1
#endif

#ifdef __cplusplus
}
#endif

union ldshape {
	long double f;
	struct {
		uint64_t m;
		uint16_t se;
	} i;
};

/* Support non-nearest rounding mode.  */
#define WANT_ROUNDING 1
/* Support signaling NaNs.  */
#define WANT_SNAN 0

#if WANT_SNAN
#error SNaN is unsupported
#else
#define issignalingf_inline(x) 0
#define issignaling_inline(x) 0
#endif

#ifndef TOINT_INTRINSICS
#define TOINT_INTRINSICS 0
#endif

#if TOINT_INTRINSICS
/* Round x to nearest int in all rounding modes, ties have to be rounded
   consistently with converttoint so the results match.  If the result
   would be outside of [-2^31, 2^31-1] then the semantics is unspecified.  */
static double_t roundtoint(double_t);

/* Convert x to nearest int in all rounding modes, ties have to be rounded
   consistently with roundtoint.  If the result is not representible in an
   int32_t then the semantics is unspecified.  */
static int32_t converttoint(double_t);
#endif

/* Helps static branch prediction so hot path can be better optimized.  */
#ifdef __GNUC__
#define predict_true(x) __builtin_expect(!!(x), 1)
#define predict_false(x) __builtin_expect(x, 0)
#else
#define predict_true(x) (x)
#define predict_false(x) (x)
#endif

/* Evaluate an expression as the specified type. With standard excess
   precision handling a type cast or assignment is enough (with
   -ffloat-store an assignment is required, in old compilers argument
   passing and return statement may not drop excess precision).  */

static inline float eval_as_float(float x)
{
	float y = x;
	return y;
}

static inline double eval_as_double(double x)
{
	double y = x;
	return y;
}

/* fp_barrier returns its input, but limits code transformations
   as if it had a side-effect (e.g. observable io) and returned
   an arbitrary value.  */

#ifndef fp_barrierf
#define fp_barrierf fp_barrierf
static inline float fp_barrierf(float x)
{
	volatile float y = x;
	return y;
}
#endif

#ifndef fp_barrier
#define fp_barrier fp_barrier
static inline double fp_barrier(double x)
{
	volatile double y = x;
	return y;
}
#endif

#ifndef fp_barrierl
#define fp_barrierl fp_barrierl
static inline long double fp_barrierl(long double x)
{
	volatile long double y = x;
	return y;
}
#endif

/* fp_force_eval ensures that the input value is computed when that's
   otherwise unused.  To prevent the constant folding of the input
   expression, an additional fp_barrier may be needed or a compilation
   mode that does so (e.g. -frounding-math in gcc). Then it can be
   used to evaluate an expression for its fenv side-effects only.   */

#ifndef fp_force_evalf
#define fp_force_evalf fp_force_evalf
static inline void fp_force_evalf(float x)
{
	volatile float y;
	y = x;
}
#endif

#ifndef fp_force_eval
#define fp_force_eval fp_force_eval
static inline void fp_force_eval(double x)
{
	volatile double y;
	y = x;
}
#endif

#ifndef fp_force_evall
#define fp_force_evall fp_force_evall
static inline void fp_force_evall(long double x)
{
	volatile long double y;
	y = x;
}
#endif

#define FORCE_EVAL(x) do {                        \
	if (sizeof(x) == sizeof(float)) {         \
		fp_force_evalf(x);                \
	} else if (sizeof(x) == sizeof(double)) { \
		fp_force_eval(x);                 \
	} else {                                  \
		fp_force_evall(x);                \
	}                                         \
} while(0)

// #define asuint(f) ((union{float _f; uint32_t _i;}){f})._i
// #define asfloat(i) ((union{uint32_t _i; float _f;}){i})._f
// #define asuint64(f) ((union{double _f; uint64_t _i;}){f})._i
// #define asdouble(i) ((union{uint64_t _i; double _f;}){i})._f

static inline uint32_t asuint(float f)      { return reinterpret_cast<uint32_t&>(f); }
static inline float    asfloat(uint32_t i)  { return reinterpret_cast<float&>(i); }
static inline uint64_t asuint64(double f)   { return reinterpret_cast<uint64_t&>(f); }
static inline double   asdouble(uint64_t i) { return reinterpret_cast<double&>(i); }

#define EXTRACT_WORDS(hi,lo,d)                    \
do {                                              \
  uint64_t __u = asuint64(d);                     \
  (hi) = __u >> 32;                               \
  (lo) = (uint32_t)__u;                           \
} while (0)

#define GET_HIGH_WORD(hi,d)                       \
do {                                              \
  (hi) = asuint64(d) >> 32;                       \
} while (0)

#define GET_LOW_WORD(lo,d)                        \
do {                                              \
  (lo) = (uint32_t)asuint64(d);                   \
} while (0)

#define INSERT_WORDS(d,hi,lo)                     \
do {                                              \
  (d) = asdouble(((uint64_t)(hi)<<32) | (uint32_t)(lo)); \
} while (0)

#define SET_HIGH_WORD(d,hi)                       \
  INSERT_WORDS(d, hi, (uint32_t)asuint64(d))

#define SET_LOW_WORD(d,lo)                        \
  INSERT_WORDS(d, asuint64(d)>>32, lo)

#define GET_FLOAT_WORD(w,d)                       \
do {                                              \
  (w) = asuint(d);                                \
} while (0)

#define SET_FLOAT_WORD(d,w)                       \
do {                                              \
  (d) = asfloat(w);                               \
} while (0)

extern int __signgam;

/* error handling functions */
// hidden float __math_xflowf(uint32_t, float);
// hidden float __math_uflowf(uint32_t);
// hidden float __math_oflowf(uint32_t);
// hidden float __math_divzerof(uint32_t);
// hidden float __math_invalidf(float);
#ifndef __math_xflow
static inline double __math_xflow(uint32_t sign, double y) {
  return eval_as_double(fp_barrier(sign ? -y : y) * y);
}
#endif
#ifndef __math_uflow
static inline double __math_uflow(uint32_t sign) {
	return __math_xflow(sign, 0x1p-767);
}
#endif
#ifndef __math_uflow
static inline double __math_oflow(uint32_t sign) {
	return __math_xflow(sign, 0x1p769);
}
#endif
#ifndef __math_divzero
static inline double __math_divzero(uint32_t sign) {
	return fp_barrier(sign ? -1.0 : 1.0) / 0.0;
}
#endif
#ifndef __math_invalid
static inline double __math_invalid(double x) {
	return (x - x) / (x - x);
}
#endif

/* Top 16 bits of a double.  */
#ifndef top16
static inline uint32_t top16(double x) {
	return asuint64(x) >> 48;
}
#endif

#include "log.c"
#include "log2.c"

#endif