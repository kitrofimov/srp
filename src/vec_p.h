// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  `vec2fp_24_6` */

#pragma once

#include <stdint.h>
#include "fixed_point.h"
#include "vec.h"

/** @ingroup Vector_internal
 *  @{ */

#pragma pack(push, 1)

/** Represents a 2D vector of `fixed_24_8`s */
typedef struct vec2fix_56_8 { fixed_56_8 x, y; } vec2fix_56_8;

#pragma pack(pop)

vec2fix_56_8 vec2fp_56_8_subtract(vec2fix_56_8 a, vec2fix_56_8 b);

