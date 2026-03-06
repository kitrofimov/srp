// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Math_internal
 *  `vec2`, `vec3`, `vec4` implementation */

#include <math.h>
#include "utils/message_callback_p.h"
#include "utils/defines.h"
#include "srp/vec.h"

/** @ingroup Math_internal
 *  @{ */

SRP_FORCEINLINE vec2 vec2Add(vec2 a, vec2 b)
{
	return VEC2(a.x + b.x, a.y + b.y);
}

SRP_FORCEINLINE vec2 vec2Subtract(vec2 a, vec2 b)
{
	return VEC2(a.x - b.x, a.y - b.y);
}

SRP_FORCEINLINE float vec2DotProduct(vec2 a, vec2 b)
{
	return a.x * b.x + a.y * b.y;
}

SRP_FORCEINLINE vec2 vec2MultiplyScalar(vec2 a, float b)
{
	return VEC2(a.x * b, a.y * b);
}

SRP_FORCEINLINE vec2 vec2Negate(vec2 v) 
{ 
    return VEC2(-v.x, -v.y); 
}

SRP_FORCEINLINE vec2 vec2MultiplyVec2(vec2 a, vec2 b) 
{ 
    return VEC2(a.x * b.x, a.y * b.y); 
}

SRP_FORCEINLINE vec2 vec2Normalize(vec2 v) 
{
    float length = sqrtf(v.x * v.x + v.y * v.y);
    if (length > 0)
	{
        float inv = 1.0f / length;
        return VEC2(v.x * inv, v.y * inv);
    }
    return VEC2(0, 0);
}

SRP_FORCEINLINE vec2 vec2Reflect(vec2 i, vec2 n) 
{
    float dot = vec2DotProduct(n, i);
    vec2 factor = vec2MultiplyScalar(n, 2.f * dot);
    return vec2Subtract(i, factor);
}


SRP_FORCEINLINE vec3 vec3Add(vec3 a, vec3 b)
{
	return VEC3(
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	);
}

SRP_FORCEINLINE vec3 vec3Subtract(vec3 a, vec3 b)
{
	return VEC3(
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
	);
}

SRP_FORCEINLINE float vec3DotProduct(vec3 a, vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

SRP_FORCEINLINE vec3 vec3MultiplyScalar(vec3 a, float b)
{
	return VEC3(
		a.x * b,
		a.y * b,
		a.z * b
	);
}

SRP_FORCEINLINE vec3 vec3Normalize(vec3 v)
{
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length > 0)
	{
		float inv = 1 / length;
        return VEC3(v.x * inv, v.y * inv, v.z * inv);
    }
    return VEC3(0, 0, 0);
}

SRP_FORCEINLINE vec3 vec3Reflect(vec3 i, vec3 n)
{
    float dot = vec3DotProduct(n, i);
    vec3 factor = vec3MultiplyScalar(n, 2.f * dot);
    return vec3Subtract(i, factor);  // R = I - 2 * dot(N, I) * N
}

SRP_FORCEINLINE vec3 vec3MultiplyVec3(vec3 a, vec3 b)
{
    return VEC3(a.x * b.x, a.y * b.y, a.z * b.z);
}

SRP_FORCEINLINE vec3 vec3Negate(vec3 v)
{
    return VEC3(-v.x, -v.y, -v.z);
}


SRP_FORCEINLINE vec4 vec4Add(vec4 a, vec4 b)
{
	return VEC4(
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
		a.w + b.w
	);
}

SRP_FORCEINLINE vec4 vec4Subtract(vec4 a, vec4 b)
{
	return VEC4(
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
		a.w - b.w
	);
}

SRP_FORCEINLINE float vec4DotProduct(vec4 a, vec4 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

SRP_FORCEINLINE vec4 vec4MultiplyScalar(vec4 a, float b)
{
	return VEC4(
		a.x * b,
		a.y * b,
		a.z * b,
		a.w * b
	);
}

SRP_FORCEINLINE vec4 vec4Negate(vec4 v) 
{ 
    return VEC4(-v.x, -v.y, -v.z, -v.w);
}

SRP_FORCEINLINE vec4 vec4MultiplyVec4(vec4 a, vec4 b) 
{ 
    return VEC4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

SRP_FORCEINLINE vec4 vec4Normalize(vec4 v) 
{
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    if (length > 0)
	{
        float inv = 1.0f / length;
        return VEC4(v.x * inv, v.y * inv, v.z * inv, v.w * inv);
    }
    return VEC4(0, 0, 0, 0);
}

SRP_FORCEINLINE vec4 vec4Reflect(vec4 i, vec4 n) 
{
    float dot = vec4DotProduct(n, i);
    vec4 factor = vec4MultiplyScalar(n, 2.f * dot);
    return vec4Subtract(i, factor);
}

/** @} */  // ingroup Math_internal
