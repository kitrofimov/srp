// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include "utils/message_callback_p.h"
#include "utils/defines.h"
#include "srp/vec.h"

SRP_FORCEINLINE vec3d vec3dSubtract(vec3d a, vec3d b)
{
	return (vec3d) {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
	};
}


SRP_FORCEINLINE vec4d vec4dAdd(vec4d a, vec4d b)
{
	return (vec4d) {
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
		a.w + b.w
	};
}

SRP_FORCEINLINE vec4d vec4dMultiplyScalar(vec4d a, double b)
{
	return (vec4d) {
		a.x * b,
		a.y * b,
		a.z * b,
		a.w * b
	};
}

SRP_FORCEINLINE double vec4dIndex(vec4d a, uint8_t index)
{
	if (index >= 4)
	{
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Attempt to OBB access vec4d: index (%i)", index
		);
		return 0;
	}
	return ((double*) &a)[index];
}

