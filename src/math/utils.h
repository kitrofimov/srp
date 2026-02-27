// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Math_internal
 *  Utility macros related to mathematics */

/** @ingroup Math_internal
 *  @{ */

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

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
/** Determine if two values are roughly equal */
#define ROUGHLY_EQUAL(a, b) (fabs((a) - (b)) <= EPSILON)
/** Determine if a value is roughly zero */
#define ROUGHLY_ZERO(x) (fabs(x) <= EPSILON)

/** Determine if the first value is roughly less or equal than the second one */
#define ROUGHLY_LESS_OR_EQUAL(a, b) ((a) <= (b) + EPSILON)
/** Determine if the first value is roughly greater or equal than the second one */
#define ROUGHLY_GREATER_OR_EQUAL(a, b) ((a) >= (b) - EPSILON)

/** @} */  // ingroup Math_internal
