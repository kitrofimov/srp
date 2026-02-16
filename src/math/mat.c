// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <math.h>
#include "utils/message_callback_p.h"
#include "utils/defines.h"
#include "srp/mat.h"

SRP_FORCEINLINE vec4d mat4dGetColumn(const mat4d* a, uint8_t index)
{
	if (index >= 4)
	{
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Attempt to OBB access mat4d (read): column index (%i)", index
		);
		return VEC4D_ZERO;
	}

	return (vec4d) {
		a->data[0][index],
		a->data[1][index],
		a->data[2][index],
		a->data[3][index]
	};
}

SRP_FORCEINLINE void mat4dSetColumn(mat4d* a, vec4d column, uint8_t index)
{
	if (index >= 4)
	{
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Attempt to OBB access mat4d (write): column index (%i)", index
		);
		return;
	}

	a->data[0][index] = column.x;
	a->data[1][index] = column.y;
	a->data[2][index] = column.z;
	a->data[3][index] = column.w;
}

SRP_FORCEINLINE vec4d mat4dMultiplyVec4d(const mat4d* a, vec4d b)
{
	vec4d res = {0};
	for (uint8_t i = 0; i < 4; i++)
	{
		res = vec4dAdd(res, vec4dMultiplyScalar(
			mat4dGetColumn(a, i), vec4dIndex(b, i)
		));
	}
	return res;
}


SRP_FORCEINLINE mat4d mat4dMultiplyMat4d(const mat4d* a, const mat4d* b)
{
	mat4d res = {0};
	for (uint8_t i = 0; i < 4; i++)
	{
		vec4d column = mat4dMultiplyVec4d(a, mat4dGetColumn(b, i));
		mat4dSetColumn(&res, column, i);
	}
	return res;
}

SRP_FORCEINLINE mat4d mat4dConstructIdentity()
{
	return (mat4d) {{
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}
	}};
}

SRP_FORCEINLINE mat4d mat4dConstructScale(double x, double y, double z)
{
	return (mat4d) {{
		{x, 0, 0, 0},
		{0, y, 0, 0},
		{0, 0, z, 0},
		{0, 0, 0, 1}
	}};
}

SRP_FORCEINLINE mat4d mat4dConstructTranslate(double x, double y, double z)
{
	return (mat4d) {{
		{1, 0, 0, x},
		{0, 1, 0, y},
		{0, 0, 1, z},
		{0, 0, 0, 1}
	}};
}

SRP_FORCEINLINE mat4d mat4dConstructRotate(double x, double y, double z)
{
	mat4d res = {0};
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

SRP_FORCEINLINE mat4d mat4dConstructTRS(
	double transX, double transY, double transZ,
	double rotataionX, double rotataionY, double rotataionZ,
	double scaleX, double scaleY, double scaleZ
)
{
	mat4d T = mat4dConstructTranslate(transX, transY, transZ);
	mat4d R = mat4dConstructRotate(rotataionX, rotataionY, rotataionZ);
	mat4d S = mat4dConstructScale(scaleX, scaleY, scaleZ);
	mat4d RS = mat4dMultiplyMat4d(&R, &S);
	return mat4dMultiplyMat4d(&T, &RS);
}

SRP_FORCEINLINE mat4d mat4dConstructView(
	double cameraX, double cameraY, double cameraZ,
	double rotataionX, double rotataionY, double rotataionZ,
	double scaleX, double scaleY, double scaleZ
)
{
	return mat4dConstructTRS(
		-cameraX, -cameraY, -cameraZ,
		-rotataionX, -rotataionY, -rotataionZ,
		scaleX, scaleY, scaleZ
	);
}

// Construct matrix that transforms everything inside specified rectangular
// parallelepiped to unit cube (min: (-1, -1, -1), max: (1, 1, 1))
SRP_FORCEINLINE mat4d mat4dConstructOrthogonalProjection(
	double x_min, double x_max,
	double y_min, double y_max,
	double z_min, double z_max
)
{
	mat4d res = {0};
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
SRP_FORCEINLINE mat4d mat4dConstructPerspectiveProjection(
	double x_min_near, double x_max_near,
	double y_min_near, double y_max_near,
	double z_near, double z_far
)
{
	mat4d perspective = {0};
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

	mat4d orthogonal = mat4dConstructOrthogonalProjection(
		x_min_near, x_max_near,
		y_min_near, y_max_near,
		z_near, z_far
	);

	return mat4dMultiplyMat4d(&orthogonal, &perspective);
}

