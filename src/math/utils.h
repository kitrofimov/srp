// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

/** @file
 *  Utility macros related to mathematics */

#define _USE_MATH_DEFINES
#include <math.h>

/** @ingroup Various_internal
 *  @{ */

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

/** Get the minimum of two values */
#define MIN(a, b) ( (a) > (b) ? (b) : (a) )
/** Get the maximum of two values */
#define MAX(a, b) ( (a) > (b) ? (a) : (b) )

/** Clamp the `value` between `min` and `max`
 *  @return `min`, if `value` < `min`;
 *          `max`, if `value` > `max`;
 *          `value` otherwise */
#define CLAMP(min, max, value) \
	( \
		(value < min) ? \
			(min) : \
			(value > max) ? \
				(max) : \
				(value) \
	)

/** Get the fractional part of `float` or `double` */
#define FRACTIONAL(x) ( (x) - floor(x) )

#define EPSILON 1e-9
#define ROUGHLY_EQUAL(a, b) (fabs((a) - (b)) <= EPSILON)
#define ROUGHLY_ZERO(x) (fabs(x) <= EPSILON)

/** @} */  // ingroup Various_internal

