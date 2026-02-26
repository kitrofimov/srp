// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

/** @file
 *  `vec2`, `vec3`, `vec4` definition */

#include <stdint.h>

/** @ingroup Vector
 *  @{ */

#pragma pack(push, 1)

/** Represents a 2D vector of `float`s */
typedef struct vec2 { float x, y; } vec2;
#define VEC2_ZERO (vec2) {0, 0}

/** Represents a 3D vector of `float`s */
typedef struct vec3 { float x, y, z; } vec3;
#define VEC3_ZERO (vec3) {0, 0, 0}

/** Represents a 4D vector of `float`s */
typedef struct vec4 { float x, y, z, w; } vec4;
#define VEC4_ZERO (vec4) {0, 0, 0, 0}

#pragma pack(pop)

/** Subtract two `vec3`s
 *  @param[in] a First vector @param[in] b Second vector
 *  @return The difference between first and second vectors */
vec3 vec3Subtract(vec3 a, vec3 b);

/** Add two `vec4`s
 *  @param[in] a First vector @param[in] b Second vector
 *  @return The sum of first and second vectors */
vec4 vec4Add(vec4 a, vec4 b);

/** Multiply a `vec4` by a `float` scalar
 *  @param[in] a Vector @param[in] b Scalar
 *  @return The result of vector-scalar multiplication */
vec4 vec4MultiplyScalar(vec4 a, float b);

/** @} */  // defgroup Vector

