// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

#include <stdint.h>
#include <limits.h>
#include <math.h>

/** @file
 *  Fixed-point math
 *  @see https://vanhunteradams.com/FixedPoint/FixedPoint.html */

typedef int32_t fixed_24_8;

#define FIXED_24_8_FROM_INT(x)     ( (x) << 8 )
#define FIXED_24_8_FROM_FLOAT(x)   ( roundf((x) * (1 << 8)) )  // * 2^8
#define FIXED_24_8_FROM_DOUBLE(x)  ( round ((x) * (1 << 8)) )  // * 2^8

#define FIXED_24_8_TO_INT(x)       ( (x) >> 8 )
#define FIXED_24_8_TO_FLOAT(x)     ( (float) (x) / (1 << 8) )   // / 2^8
#define FIXED_24_8_TO_DOUBLE(x)    ( (double) (x) / (1 << 8) )  // / 2^8

#define FIXED_24_8_MIN_VALUE 1
#define FIXED_24_8_ONE             FIXED_24_8_FROM_INT(1)
#define FIXED_24_8_INTEGER_MASK    0xFFFFFFFFFFFFFF00
#define FIXED_24_8_FRACTIONAL_MASK 0x00000000000000FF

#define FIXED_24_8_FLOOR(x)        ( (x) & FIXED_24_8_INTEGER_MASK )
#define FIXED_24_8_FRACTIONAL(x)   ( (x) & FIXED_24_8_FRACTIONAL_MASK )

#define FIXED_24_8_MULTIPLY(x, y)  ( (fixed_24_8) (((int64_t) (x) * (int64_t) (y)) >> 8) )

