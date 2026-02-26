// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include "utils/message_callback_p.h"
#include "utils/defines.h"
#include "srp/vec.h"

SRP_FORCEINLINE vec2 vec2Add(vec2 a, vec2 b)
{
	return (vec2) {
		a.x + b.x,
		a.y + b.y
	};
}

SRP_FORCEINLINE vec2 vec2Subtract(vec2 a, vec2 b)
{
	return (vec2) {
		a.x - b.x,
		a.y - b.y
	};
}

SRP_FORCEINLINE float vec2DotProduct(vec2 a, vec2 b)
{
	return a.x * b.x + a.y * b.y;
}

SRP_FORCEINLINE vec2 vec2MultiplyScalar(vec2 a, float b)
{
	return (vec2) {
		a.x * b,
		a.y * b
	};
}

SRP_FORCEINLINE vec3 vec3Add(vec3 a, vec3 b)
{
	return (vec3) {
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	};
}

SRP_FORCEINLINE vec3 vec3Subtract(vec3 a, vec3 b)
{
	return (vec3) {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
	};
}

SRP_FORCEINLINE float vec3DotProduct(vec3 a, vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

SRP_FORCEINLINE vec3 vec3MultiplyScalar(vec3 a, float b)
{
	return (vec3) {
		a.x * b,
		a.y * b,
		a.z * b
	};
}

SRP_FORCEINLINE vec4 vec4Add(vec4 a, vec4 b)
{
	return (vec4) {
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
		a.w + b.w
	};
}

SRP_FORCEINLINE vec4 vec4Subtract(vec4 a, vec4 b)
{
	return (vec4) {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
		a.w - b.w
	};
}

SRP_FORCEINLINE float vec4DotProduct(vec4 a, vec4 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

SRP_FORCEINLINE vec4 vec4MultiplyScalar(vec4 a, float b)
{
	return (vec4) {
		a.x * b,
		a.y * b,
		a.z * b,
		a.w * b
	};
}
