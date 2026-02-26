// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <math.h>
#include "utils/message_callback_p.h"
#include "utils/defines.h"
#include "srp/mat.h"

SRP_FORCEINLINE vec4 mat4MultiplyVec4(const mat4* m, vec4 v)
{
    vec4 r;

    r.x =
        m->data[0][0] * v.x +
        m->data[0][1] * v.y +
        m->data[0][2] * v.z +
        m->data[0][3] * v.w;

    r.y =
        m->data[1][0] * v.x +
        m->data[1][1] * v.y +
        m->data[1][2] * v.z +
        m->data[1][3] * v.w;

    r.z =
        m->data[2][0] * v.x +
        m->data[2][1] * v.y +
        m->data[2][2] * v.z +
        m->data[2][3] * v.w;

    r.w =
        m->data[3][0] * v.x +
        m->data[3][1] * v.y +
        m->data[3][2] * v.z +
        m->data[3][3] * v.w;

    return r;
}

SRP_FORCEINLINE mat4 mat4MultiplyMat4(const mat4* a, const mat4* b)
{
    mat4 r;
	const float (*A)[4] = a->data;
    const float (*B)[4] = b->data;
    float (*R)[4] = r.data;

    R[0][0] = A[0][0] * B[0][0] + A[0][1] * B[1][0] + A[0][2] * B[2][0] + A[0][3] * B[3][0];
    R[0][1] = A[0][0] * B[0][1] + A[0][1] * B[1][1] + A[0][2] * B[2][1] + A[0][3] * B[3][1];
    R[0][2] = A[0][0] * B[0][2] + A[0][1] * B[1][2] + A[0][2] * B[2][2] + A[0][3] * B[3][2];
    R[0][3] = A[0][0] * B[0][3] + A[0][1] * B[1][3] + A[0][2] * B[2][3] + A[0][3] * B[3][3];

    R[1][0] = A[1][0] * B[0][0] + A[1][1] * B[1][0] + A[1][2] * B[2][0] + A[1][3] * B[3][0];
    R[1][1] = A[1][0] * B[0][1] + A[1][1] * B[1][1] + A[1][2] * B[2][1] + A[1][3] * B[3][1];
    R[1][2] = A[1][0] * B[0][2] + A[1][1] * B[1][2] + A[1][2] * B[2][2] + A[1][3] * B[3][2];
    R[1][3] = A[1][0] * B[0][3] + A[1][1] * B[1][3] + A[1][2] * B[2][3] + A[1][3] * B[3][3];

    R[2][0] = A[2][0] * B[0][0] + A[2][1] * B[1][0] + A[2][2] * B[2][0] + A[2][3] * B[3][0];
    R[2][1] = A[2][0] * B[0][1] + A[2][1] * B[1][1] + A[2][2] * B[2][1] + A[2][3] * B[3][1];
    R[2][2] = A[2][0] * B[0][2] + A[2][1] * B[1][2] + A[2][2] * B[2][2] + A[2][3] * B[3][2];
    R[2][3] = A[2][0] * B[0][3] + A[2][1] * B[1][3] + A[2][2] * B[2][3] + A[2][3] * B[3][3];

    R[3][0] = A[3][0] * B[0][0] + A[3][1] * B[1][0] + A[3][2] * B[2][0] + A[3][3] * B[3][0];
    R[3][1] = A[3][0] * B[0][1] + A[3][1] * B[1][1] + A[3][2] * B[2][1] + A[3][3] * B[3][1];
    R[3][2] = A[3][0] * B[0][2] + A[3][1] * B[1][2] + A[3][2] * B[2][2] + A[3][3] * B[3][2];
    R[3][3] = A[3][0] * B[0][3] + A[3][1] * B[1][3] + A[3][2] * B[2][3] + A[3][3] * B[3][3];

    return r;
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

