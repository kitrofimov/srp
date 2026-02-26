// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <math.h>
#include "utils/message_callback_p.h"
#include "utils/defines.h"
#include "srp/mat.h"

SRP_FORCEINLINE vec4 mat4GetColumn(const mat4* a, uint8_t index)
{
	if (index >= 4)
	{
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Attempt to OBB access mat4 (read): column index (%i)", index
		);
		return VEC4_ZERO;
	}

	return (vec4) {
		a->data[0][index],
		a->data[1][index],
		a->data[2][index],
		a->data[3][index]
	};
}

SRP_FORCEINLINE void mat4SetColumn(mat4* a, vec4 column, uint8_t index)
{
	if (index >= 4)
	{
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Attempt to OBB access mat4 (write): column index (%i)", index
		);
		return;
	}

	a->data[0][index] = column.x;
	a->data[1][index] = column.y;
	a->data[2][index] = column.z;
	a->data[3][index] = column.w;
}

SRP_FORCEINLINE vec4 mat4MultiplyVec4(const mat4* a, vec4 b)
{
	vec4 res = {0};
	for (uint8_t i = 0; i < 4; i++)
	{
		res = vec4Add(res, vec4MultiplyScalar(
			mat4GetColumn(a, i), vec4Index(b, i)
		));
	}
	return res;
}


SRP_FORCEINLINE mat4 mat4MultiplyMat4(const mat4* a, const mat4* b)
{
	mat4 res = {0};
	for (uint8_t i = 0; i < 4; i++)
	{
		vec4 column = mat4MultiplyVec4(a, mat4GetColumn(b, i));
		mat4SetColumn(&res, column, i);
	}
	return res;
}

SRP_FORCEINLINE mat4 mat4ConstructIdentity()
{
	return (mat4) {{
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}
	}};
}

SRP_FORCEINLINE mat4 mat4ConstructScale(float x, float y, float z)
{
	return (mat4) {{
		{x, 0, 0, 0},
		{0, y, 0, 0},
		{0, 0, z, 0},
		{0, 0, 0, 1}
	}};
}

SRP_FORCEINLINE mat4 mat4ConstructTranslate(float x, float y, float z)
{
	return (mat4) {{
		{1, 0, 0, x},
		{0, 1, 0, y},
		{0, 0, 1, z},
		{0, 0, 0, 1}
	}};
}

SRP_FORCEINLINE mat4 mat4ConstructRotate(float x, float y, float z)
{
	mat4 res = {0};
	res.data[0][0] = cos(y) * cos(z);
	res.data[0][1] = sin(x) * sin(y) * cos(z) - cos(x) * sin(z);
	res.data[0][2] = cos(x) * sin(y) * cos(z) + sin(x) * sin(z);
	res.data[0][3] = 0;
	res.data[1][0] = cos(y) * sin(z);
	res.data[1][1] = sin(x) * sin(y) * sin(z) + cos(x) * cos(z);
	res.data[1][2] = cos(x) * sin(y) * sin(z) - sin(x) * cos(z);
	res.data[1][3] = 0;
	res.data[2][0] = -sin(y);
	res.data[2][1] = sin(x) * cos(y);
	res.data[2][2] = cos(x) * cos(y);
	res.data[2][3] = 0;
	res.data[3][0] = 0;
	res.data[3][1] = 0;
	res.data[3][2] = 0;
	res.data[3][3] = 1;
	return res;
}

SRP_FORCEINLINE mat4 mat4ConstructTRS(
	float transX, float transY, float transZ,
	float rotataionX, float rotataionY, float rotataionZ,
	float scaleX, float scaleY, float scaleZ
)
{
	mat4 T = mat4ConstructTranslate(transX, transY, transZ);
	mat4 R = mat4ConstructRotate(rotataionX, rotataionY, rotataionZ);
	mat4 S = mat4ConstructScale(scaleX, scaleY, scaleZ);
	mat4 RS = mat4MultiplyMat4(&R, &S);
	return mat4MultiplyMat4(&T, &RS);
}

SRP_FORCEINLINE mat4 mat4ConstructView(
	float cameraX, float cameraY, float cameraZ,
	float rotataionX, float rotataionY, float rotataionZ,
	float scaleX, float scaleY, float scaleZ
)
{
	return mat4ConstructTRS(
		-cameraX, -cameraY, -cameraZ,
		-rotataionX, -rotataionY, -rotataionZ,
		scaleX, scaleY, scaleZ
	);
}

// Construct matrix that transforms everything inside specified rectangular
// parallelepiped to unit cube (min: (-1, -1, -1), max: (1, 1, 1))
SRP_FORCEINLINE mat4 mat4ConstructOrthogonalProjection(
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
)
{
	mat4 res = {0};
	res.data[0][0] = 2 / (x_max - x_min);
	res.data[0][1] = 0;
	res.data[0][2] = 0;
	res.data[0][3] = -(x_max + x_min) / (x_max - x_min);
	res.data[1][0] = 0;
	res.data[1][1] = 2 / (y_max - y_min);
	res.data[1][2] = 0;
	res.data[1][3] = -(y_max + y_min) / (y_max - y_min);
	res.data[2][0] = 0;
	res.data[2][1] = 0;
	res.data[2][2] = 2 / (z_max - z_min);
	res.data[2][3] = -(z_max + z_min) / (z_max - z_min);
	res.data[3][0] = 0;
	res.data[3][1] = 0;
	res.data[3][2] = 0;
	res.data[3][3] = 1;
	return res;
}

/** @todo Avoid matmul here? */
SRP_FORCEINLINE mat4 mat4ConstructPerspectiveProjection(
	float x_min_near, float x_max_near,
	float y_min_near, float y_max_near,
	float z_near, float z_far
)
{
	mat4 perspective = {0};
	perspective.data[0][0] = z_near;
	perspective.data[0][1] = 0;
	perspective.data[0][2] = 0;
	perspective.data[0][3] = 0;
	perspective.data[1][0] = 0;
	perspective.data[1][1] = z_near;
	perspective.data[1][2] = 0;
	perspective.data[1][3] = 0;
	perspective.data[2][0] = 0;
	perspective.data[2][1] = 0;
	perspective.data[2][2] = z_near + z_far;
	perspective.data[2][3] = -z_near * z_far;
	perspective.data[3][0] = 0;
	perspective.data[3][1] = 0;
	perspective.data[3][2] = 1;
	perspective.data[3][3] = 0;

	mat4 orthogonal = mat4ConstructOrthogonalProjection(
		x_min_near, x_max_near,
		y_min_near, y_max_near,
		z_near, z_far
	);

	return mat4MultiplyMat4(&orthogonal, &perspective);
}

