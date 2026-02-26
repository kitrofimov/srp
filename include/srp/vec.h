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

vec2 vec2Add(vec2 a, vec2 b);
vec2 vec2Subtract(vec2 a, vec2 b);
float vec2DotProduct(vec2 a, vec2 b);
vec2 vec2MultiplyScalar(vec2 a, float b);

vec3 vec3Add(vec3 a, vec3 b);
vec3 vec3Subtract(vec3 a, vec3 b);
float vec3DotProduct(vec3 a, vec3 b);
vec3 vec3MultiplyScalar(vec3 a, float b);

vec4 vec4Add(vec4 a, vec4 b);
vec4 vec4Subtract(vec4 a, vec4 b);
float vec4DotProduct(vec4 a, vec4 b);
vec4 vec4MultiplyScalar(vec4 a, float b);

/** @} */  // defgroup Vector

