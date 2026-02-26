// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include "utils/message_callback_p.h"
#include "utils/defines.h"
#include "srp/vec.h"

SRP_FORCEINLINE vec3 vec3Subtract(vec3 a, vec3 b)
{
	return (vec3) {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
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

SRP_FORCEINLINE vec4 vec4MultiplyScalar(vec4 a, float b)
{
	return (vec4) {
		a.x * b,
		a.y * b,
		a.z * b,
		a.w * b
	};
}
