// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Math
 *  `vec2`, `vec3`, `vec4` definition */

#pragma once

#include <stdint.h>

/** @ingroup Math
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

/** Add two vectors */
vec2 vec2Add(vec2 a, vec2 b);
/** Subtract two vectors */
vec2 vec2Subtract(vec2 a, vec2 b);
/** Calculate the dot product of two vectors */
float vec2DotProduct(vec2 a, vec2 b);
/** Multiply a vector with a scalar value */
vec2 vec2MultiplyScalar(vec2 a, float b);
/** Normalize a vector to have a length of 1 */
vec2 vec2Normalize(vec2 v);
/** Reflect an incident vector I against a surface normal N */
vec2 vec2Reflect(vec2 i, vec2 n);
/** Component-wise multiplication (Hadamard product) */
vec2 vec2MultiplyVec2(vec2 a, vec2 b);
/** Negate all components of a vector */
vec2 vec2Negate(vec2 v);

/** Add two vectors */
vec3 vec3Add(vec3 a, vec3 b);
/** Subtract two vectors */
vec3 vec3Subtract(vec3 a, vec3 b);
/** Calculate the dot product of two vectors */
float vec3DotProduct(vec3 a, vec3 b);
/** Multiply a vector with a scalar value */
vec3 vec3MultiplyScalar(vec3 a, float b);
/** Normalize a vector to have a length of 1 */
vec3 vec3Normalize(vec3 v);
/** Reflect an incident vector I against a surface normal N */
vec3 vec3Reflect(vec3 i, vec3 n);
/** Component-wise multiplication (Hadamard product) */
vec3 vec3MultiplyVec3(vec3 a, vec3 b);
/** Negate all components of a vector */
vec3 vec3Negate(vec3 v);

/** Add two vectors */
vec4 vec4Add(vec4 a, vec4 b);
/** Subtract two vectors */
vec4 vec4Subtract(vec4 a, vec4 b);
/** Calculate the dot product of two vectors */
float vec4DotProduct(vec4 a, vec4 b);
/** Multiply a vector with a scalar value */
vec4 vec4MultiplyScalar(vec4 a, float b);
/** Normalize a vector to have a length of 1 */
vec4 vec4Normalize(vec4 v);
/** Reflect an incident vector I against a surface normal N */
vec4 vec4Reflect(vec4 i, vec4 n);
/** Component-wise multiplication (Hadamard product) */
vec4 vec4MultiplyVec4(vec4 a, vec4 b);
/** Negate all components of a vector */
vec4 vec4Negate(vec4 v);

/** @} */  // ingroup Math
