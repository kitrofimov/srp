// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Math_internal
 *  `mat4` implementation */

#include <math.h>
#include "utils/message_callback_p.h"
#include "utils/defines.h"
#include "math/utils.h"
#include "srp/mat.h"

/** @ingroup Math_internal
 *  @{ */

SRP_FORCEINLINE vec4 mat4MultiplyVec4(const mat4* restrict m, vec4 v)
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

SRP_FORCEINLINE mat4 mat4MultiplyMat4(const mat4* restrict a, const mat4* restrict b)
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

SRP_FORCEINLINE mat4 mat4Identity()
{
	return (mat4) {{
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}
	}};
}

SRP_FORCEINLINE mat4 mat4Scale(float x, float y, float z)
{
	return (mat4) {{
		{x, 0, 0, 0},
		{0, y, 0, 0},
		{0, 0, z, 0},
		{0, 0, 0, 1}
	}};
}

SRP_FORCEINLINE mat4 mat4Translate(float x, float y, float z)
{
	return (mat4) {{
		{1, 0, 0, x},
		{0, 1, 0, y},
		{0, 0, 1, z},
		{0, 0, 0, 1}
	}};
}

mat4 mat4Rotate(float angle, vec3 axis)
{
    const float c = cosf(angle);
    const float s = sinf(angle);
    const float t = 1.f - c;
    const vec3 a = vec3Normalize(axis);
	const float x = a.x;
	const float y = a.y;
	const float z = a.z;

    mat4 res = mat4Identity();
    res.data[0][0] = t * x*x + c;
    res.data[0][1] = t * x*y - s*z;
    res.data[0][2] = t * x*z + s*y;
    res.data[1][0] = t * x*y + s*z;
    res.data[1][1] = t * y*y + c;
    res.data[1][2] = t * y*z - s*x;
    res.data[2][0] = t * x*z - s*y;
    res.data[2][1] = t * y*z + s*x;
    res.data[2][2] = t * z*z + c;
    return res;
}

/** @todo get rid of this */
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
	mat4 T = mat4Translate(transX, transY, transZ);
	mat4 R = mat4ConstructRotate(rotataionX, rotataionY, rotataionZ);
	mat4 S = mat4Scale(scaleX, scaleY, scaleZ);
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

SRP_FORCEINLINE mat4 mat4LookAt(vec3 eye, vec3 target, vec3 up)
{
    vec3 f = vec3Normalize(vec3Subtract(eye, target));  // forward
    vec3 l = vec3Normalize(vec3CrossProduct(up, f));    // right
    vec3 u = vec3CrossProduct(f, l);                    // up

    mat4 res = mat4Identity();
    
    // Rotation part
    res.data[0][0] = l.x;  res.data[0][1] = l.y;  res.data[0][2] = l.z;
    res.data[1][0] = u.x;  res.data[1][1] = u.y;  res.data[1][2] = u.z;
    res.data[2][0] = f.x;  res.data[2][1] = f.y;  res.data[2][2] = f.z;

    // Translation part
    res.data[0][3] = -vec3DotProduct(l, eye);
    res.data[1][3] = -vec3DotProduct(u, eye);
    res.data[2][3] = -vec3DotProduct(f, eye);

    return res;
}

SRP_FORCEINLINE mat4 mat4ConstructOrthogonalProjection(
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
)
{
	const float l = x_min;  // left
	const float r = x_max;  // right
	const float b = y_min;  // bottom
	const float t = y_max;  // top
	const float n = z_min;  // near
	const float f = z_max;  // far

	mat4 res = {0};
	res.data[0][0] = 2 / (r-l);
	res.data[0][1] = 0;
	res.data[0][2] = 0;
	res.data[0][3] = -(r+l) / (r-l);
	res.data[1][0] = 0;
	res.data[1][1] = 2 / (t-b);
	res.data[1][2] = 0;
	res.data[1][3] = -(t+b) / (t-b);
	res.data[2][0] = 0;
	res.data[2][1] = 0;
	res.data[2][2] = -2 / (f-n);
	res.data[2][3] = -(f+n) / (f-n);
	res.data[3][0] = 0;
	res.data[3][1] = 0;
	res.data[3][2] = 0;
	res.data[3][3] = 1;
	return res;
}

SRP_FORCEINLINE mat4 mat4ConstructPerspectiveProjection(
	float x_min_near, float x_max_near,
	float y_min_near, float y_max_near,
	float z_near, float z_far
)
{
	const float l = x_min_near;  // left
	const float r = x_max_near;  // right
	const float b = y_min_near;  // bottom
	const float t = y_max_near;  // top
	const float n = z_near;      // near
	const float f = z_far;       // far

	mat4 perspective = {0};
	perspective.data[0][0] = 2*n / (r-l);
	perspective.data[0][1] = 0;
	perspective.data[0][2] = (r+l) / (r-l);
	perspective.data[0][3] = 0;
	perspective.data[1][0] = 0;
	perspective.data[1][1] = 2*n / (t-b);
	perspective.data[1][2] = (t+b) / (t-b);
	perspective.data[1][3] = 0;
	perspective.data[2][0] = 0;
	perspective.data[2][1] = 0;
	perspective.data[2][2] = -(f+n) / (f-n);
	perspective.data[2][3] = -2*f*n / (f-n);
	perspective.data[3][0] = 0;
	perspective.data[3][1] = 0;
	perspective.data[3][2] = -1;
	perspective.data[3][3] = 0;
	return perspective;
}

mat4 mat4ConstructPerspectiveProjectionFOV(
	float fovX, float aspectRatio, float z_near, float z_far
)
{
    float tangent = tanf(RAD(fovX / 2));
    float right = z_near * tangent;   // half width of near plane
    float top = right / aspectRatio;  // half height of near plane
	return mat4ConstructPerspectiveProjection(-right, right, -top, top, z_near, z_far);
}

/** @} */  // ingroup Math_internal
