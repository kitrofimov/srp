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

/** Represents a 2-element vector of `float`s */
typedef union vec2 {
    struct { float x, y; };
    float v[2];
} vec2;

/** Represents a 3-element vector of `float`s */
typedef union vec3 {
    struct { float x, y, z; };
    struct { vec2 xy; float _z; };
    struct { float _x; vec2 yz; };
    float v[3];
} vec3;

/** Represents a 4-element vector of `float`s */
typedef union vec4 {
    struct { float x, y, z, w; };
    struct { vec2 xy; float _z, _w; };
    struct { float _x; vec2 yz; float __w; };
    struct { float __x, _y; vec2 zw; };
    struct { vec3 xyz; float ___w; };
    struct { float ___x; vec3 yzw; };
    float v[4];
} vec4;

/** Instantiation macros */
#define VEC2(x, y)           ((vec2) {{x, y}})
#define VEC3(x, y, z)        ((vec3) {{x, y, z}})
#define VEC4(x, y, z, w)     ((vec4) {{x, y, z, w}})
#define VEC4_FROM_VEC3(v, a) ((vec4) {.xyz = (v), .___w = (a)})

/** Swizzle macros */
#define SWZ2(v, a, b)       ((vec2) {{(v).a, (v).b}})
#define SWZ3(v, a, b, c)    ((vec3) {{(v).a, (v).b, (v).c}})
#define SWZ4(v, a, b, c, d) ((vec4) {{(v).a, (v).b, (v).c, (v).d}})

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
