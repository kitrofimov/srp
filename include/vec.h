// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

/** @file
 *  `vec2`, vec2d, vec3, vec3d, vec4, vec4d */

/** @todo macros to cast vectors to arrays? */

#include <stdint.h>

/** @ingroup Vector
 *  @{ */

#pragma pack(push, 1)

/** Represents a 2D vector of `float`s */
typedef struct vec2 { float x, y; } vec2;
/** Represents a 2D vector of `double`s */
typedef struct vec2d { double x, y; } vec2d;
/** Represents a 2D vector of `int`s */
typedef struct vec2i { int x, y; } vec2i;
#define VEC2_ZERO (vec2) {0, 0}
#define VEC2D_ZERO (vec2d) {0, 0}
#define VEC2I_ZERO (vec2i) {0, 0}

/** Represents a 3D vector of `float`s */
typedef struct vec3 { float x, y, z; } vec3;
/** Represents a 3D vector of `double`s */
typedef struct vec3d { double x, y, z; } vec3d;
#define VEC3_ZERO (vec3) {0, 0, 0}
#define VEC3D_ZERO (vec3d) {0, 0, 0}

/** Represents a 4D vector of `float`s */
typedef struct vec4 { float x, y, z, w; } vec4;
/** Represents a 4D vector of `double`s */
typedef struct vec4d { double x, y, z, w; } vec4d;
#define VEC4_ZERO (vec4) {0, 0, 0, 0}
#define VEC4D_ZERO (vec4d) {0, 0, 0, 0}

#pragma pack(pop)

/** Subtract two `vec3d`s
 *  @param[in] a First vector @param[in] b Second vector
 *  @return The difference between first and second vectors */
vec3d vec3dSubtract(vec3d a, vec3d b);

/** Add two `vec4d`s
 *  @param[in] a First vector @param[in] b Second vector
 *  @return The sum of first and second vectors */
vec4d vec4dAdd(vec4d a, vec4d b);
/** Multiply a `vec4d` by a `double` scalar
 *  @param[in] a Vector @param[in] b Scalar
 *  @return The result of vector-scalar multiplication */
vec4d vec4dMultiplyScalar(vec4d a, double b);
/** Index a `vec4d`
 *  @param[in] a Vector to be indexed @param[in] index Index (0-based)
 *  @return The value stored at `index`th element of the vector */
double vec4dIndex(vec4d a, uint8_t index);

/** @} */  // defgroup Vector

