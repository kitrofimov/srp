// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "srp/context.h"
#include "srp/vertex.h"
#include "srp/shaders.h"
#include "srp/color.h"
#include "srp/vec.h"
#include "raster/triangle.h"
#include "raster/fragment.h"
#include "core/framebuffer_p.h"
#include "pipeline/vertex_processing.h"
#include "pipeline/interpolation.h"
#include "math/utils.h"
#include "utils/message_callback_p.h"
#include "utils/voidptr.h"

/** @file
 *  Triangle rasteization & data interpolation */

/** @ingroup Rasterization
 *  @{ */

/** Get a parallelogram's signed area. The two vectors define a parallelogram.
 *  Used for barycentric coordinates' initialization in
 *  calculateBarycentrics() */
static double signedAreaParallelogram(
	const vec3d* restrict a, const vec3d* restrict b
);

/** Determine if a triangle should be culled (back-face culling)
 *  @param[in] tri Triangle to check. Must have its `p_ndc` field initialized.
 *  @param[out] isCCW Whether or not the triangle's vertices are in counter-clockwise order
 *  @param[out] isFrontFacing Whether or not the triangle is front facing 
 *  @return Whether or not the triangle should be culled */
static bool shouldCullTriangle(const SRPTriangle* tri, bool* isCCW, bool* isFrontFacing);

/** Change the winding order of a triangle.
 *  @param[in] tri Triangle to change the winding order of. Must have its `v`,
 * 				   `p_ndc` and `invW` fields initialized. */
static void triangleChangeWinding(SRPTriangle* tri);

/** Calculate barycentric coordinates for a point and barycentric coordinates'
 *  delta values.
 *  @param[in] tri Triangle to calculate barycentric coordinates for. Must have
 * 				   its `ss` and `edge` fields initialized.
 *  @param[in] areaX2 The triangle's area multiplied by 2 (to avoid division)
 *  @param[in] point Point to calculate barycentric coordinates for (screen-space) */
static void calculateBarycentrics(SRPTriangle* tri, double areaX2, vec2d point);

/** Check if a triangle's edge is flat top or left. Assumes counter-clockwise
 *  vertex order!
 *  @param[in] edge A pointer to an edge vector (pointing from one vertex
 *  to the other)
 *  @return Whether or not this edge is flat top or left */
static bool isEdgeFlatTopOrLeft(const vec3d* restrict edge);

/** Interpolate the fragment position and vertex variables inside the triangle.
 *  @param[in] tri Triangle to interpolate data for
 *  @param[in] sp A pointer to shader program to use
 *  @param[out] pPosition A pointer to vec4d where interpolated position will appear.
 *  @param[out] pInterpolatedBuffer A pointer to the buffer where interpolated
 *              variables will appear. Must be big enough to hold all of them */
static void triangleInterpolateData(
	SRPTriangle* tri, const SRPShaderProgram* restrict sp,
	vec4d* pPosition, SRPInterpolated* pInterpolatedBuffer
);

/** Interpolate the fragment position inside the triangle.
 *  @param[in] tri Triangle to interpolate position for
 *  @param[out] pPosition A pointer to vec4d where interpolated position will appear. */
static void triangleInterpolatePosition(SRPTriangle* tri, vec4d* pPosition);

/** Interpolate vertex attributes inside the triangle.
 *  @param[in] tri Triangle to interpolate attributes for
 *  @param[in] sp A pointer to shader program to use
 *  @param[in] pPosition A pointer to the interpolated position
 *  @param[out] pInterpolatedBuffer A pointer to the buffer where interpolated
 *              variables will appear. Must be big enough to hold all of them */
static void triangleInterpolateAttributes(
	SRPTriangle* tri, const SRPShaderProgram* restrict sp,
	vec4d* pPosition, SRPInterpolated* pInterpolatedBuffer
);

/** @} */  // ingroup Rasterization

void rasterizeTriangle(
	SRPTriangle* tri, const SRPFramebuffer* fb,
	const SRPShaderProgram* restrict sp, void* interpolatedBuffer
)
{
	for (size_t y = tri->minBP.y; y < tri->maxBP.y; y += 1)
	{
		for (size_t x = tri->minBP.x; x < tri->maxBP.x; x += 1)
		{
			for (uint8_t i = 0; i < 3; i++)  // Top-left rasterization rule
			{
				if (ROUGHLY_ZERO(tri->lambda[i]) && !tri->edgeTL[i])
					goto nextPixel;
			}

			if (tri->lambda[0] >= 0 && tri->lambda[1] >= 0 && tri->lambda[2] >= 0)
			{
				vec4d interpolatedPosition = {0};
				triangleInterpolateData(tri, sp, &interpolatedPosition, interpolatedBuffer);

				SRPfsInput fsIn = {
					.uniform = sp->uniform,
					.interpolated = interpolatedBuffer,
					.fragCoord = {
						x + 0.5,
						y + 0.5,
						interpolatedPosition.z,
						interpolatedPosition.w
					},
					.frontFacing = tri->isFrontFacing,
					.primitiveID = tri->id,
				};
				emitFragment(fb, sp, x, y, &fsIn);
			}

nextPixel:
			for (uint8_t i = 0; i < 3; i++)
				tri->lambda[i] += tri->dldx[i];
		}
		for (uint8_t i = 0; i < 3; i++)
		{
			tri->lambda_row[i] += tri->dldy[i];
			tri->lambda[i] = tri->lambda_row[i];
		}
	}
}

bool setupTriangle(SRPTriangle* tri, const SRPFramebuffer* fb)
{
	for (uint8_t i = 0; i < 3; i++)
		applyPerspectiveDivide(&tri->v[i], &tri->invW[i]);

	// vec3d is tightly packed, so this is safe
	for (uint8_t i = 0; i < 3; i++)
		tri->p_ndc[i] = (vec3d*) tri->v[i].position;

	bool isCCW;
	if (shouldCullTriangle(tri, &isCCW, &tri->isFrontFacing))
		return false;

	if (!isCCW)
		triangleChangeWinding(tri);

	for (size_t i = 0; i < 3; i++)
		framebufferNDCToScreenSpace(fb, (double*) tri->p_ndc[i], (double*) &tri->ss[i]);

	for (size_t i = 0; i < 3; i++)
		tri->edge[i] = vec3dSubtract(tri->ss[(i+1) % 3], tri->ss[i]);

	// Cull degenerate triangles (using SS area!)
	double areaX2 = fabs(signedAreaParallelogram(&tri->edge[0], &tri->edge[2]));
	if (ROUGHLY_ZERO(areaX2))
		return false;

	tri->minBP = (vec2d) {
		floor(MIN(tri->ss[0].x, MIN(tri->ss[1].x, tri->ss[2].x))),
		floor(MIN(tri->ss[0].y, MIN(tri->ss[1].y, tri->ss[2].y)))
	};
	tri->maxBP = (vec2d) {
		ceil(MAX(tri->ss[0].x, MAX(tri->ss[1].x, tri->ss[2].x))),
		ceil(MAX(tri->ss[0].y, MAX(tri->ss[1].y, tri->ss[2].y)))
	};

	calculateBarycentrics(tri, areaX2, (vec2d) {tri->minBP.x + 0.5, tri->minBP.y + 0.5});

	for (uint8_t i = 0; i < 3; i++)
	{
		tri->lambda_row[i] = tri->lambda[i];
		tri->edgeTL[i] = isEdgeFlatTopOrLeft(&tri->edge[i]);
	}

	return true;
}

static bool shouldCullTriangle(const SRPTriangle* tri, bool* isCCW, bool* isFrontFacing)
{
	vec3d edge0 = vec3dSubtract(*tri->p_ndc[1], *tri->p_ndc[0]);
	vec3d edge1 = vec3dSubtract(*tri->p_ndc[2], *tri->p_ndc[0]);
	double signedArea = signedAreaParallelogram(&edge0, &edge1);
	*isCCW = signedArea > 0;

	// Should have already been handled, but just in case
	if (srpContext.cullFace == SRP_CULL_FACE_FRONT_AND_BACK)
		return true;

	bool frontFacing = \
		(signedArea > 0) & (srpContext.frontFace == SRP_FRONT_FACE_CCW) ||
		(signedArea < 0) & (srpContext.frontFace == SRP_FRONT_FACE_CW);
	bool cull = \
		( frontFacing && srpContext.cullFace == SRP_CULL_FACE_FRONT) ||
		(!frontFacing && srpContext.cullFace == SRP_CULL_FACE_BACK);

	*isFrontFacing = frontFacing;
	return cull;
}

static void triangleChangeWinding(SRPTriangle* tri)
{
	// Not swapping `p_ndc`, because those are pointers to `v.position`
	// If swapped both at the same time, it's almost the same as not swapping anything
	SRPvsOutput temp1 = tri->v[1];
	tri->v[1] = tri->v[2];
	tri->v[2] = temp1;

	double temp2 = tri->invW[1];
	tri->invW[1] = tri->invW[2];
	tri->invW[2] = temp2;
}

static void calculateBarycentrics(SRPTriangle* tri, double areaX2, vec2d point)
{
	vec3d AP = {
		point.x - tri->ss[0].x,
		point.y - tri->ss[0].y,
		0
	};
	vec3d BP = {
		point.x - tri->ss[1].x,
		point.y - tri->ss[1].y,
		0
	};
	vec3d CP = {
		point.x - tri->ss[2].x,
		point.y - tri->ss[2].y,
		0
	};

	tri->lambda[0] = signedAreaParallelogram(&BP, &tri->edge[1]) / areaX2;
	tri->lambda[1] = signedAreaParallelogram(&CP, &tri->edge[2]) / areaX2;
	tri->lambda[2] = signedAreaParallelogram(&AP, &tri->edge[0]) / areaX2;

	tri->dldx[0] = tri->edge[1].y / areaX2;
	tri->dldx[1] = tri->edge[2].y / areaX2;
	tri->dldx[2] = tri->edge[0].y / areaX2;

	tri->dldy[0] = -tri->edge[1].x / areaX2;
	tri->dldy[1] = -tri->edge[2].x / areaX2;
	tri->dldy[2] = -tri->edge[0].x / areaX2;
}

static double signedAreaParallelogram(
	const vec3d* restrict a, const vec3d* restrict b
)
{
	return a->x * b->y - a->y * b->x;
}

static bool isEdgeFlatTopOrLeft(const vec3d* restrict edge)
{
	return ((edge->x > 0) && ROUGHLY_ZERO(edge->y)) || (edge->y < 0);
}

static void triangleInterpolateData(
	SRPTriangle* tri, const SRPShaderProgram* restrict sp,
	vec4d* pPosition, SRPInterpolated* pInterpolatedBuffer
)
{
	triangleInterpolatePosition(tri, pPosition);
	triangleInterpolateAttributes(tri, sp, pPosition, pInterpolatedBuffer);
}

static void triangleInterpolatePosition(SRPTriangle* tri, vec4d* pPosition)
{
	bool perspective = (srpContext.interpolationMode == SRP_INTERPOLATION_MODE_PERSPECTIVE);

	pPosition->x = \
		tri->v[0].position[0] * tri->lambda[0] + \
		tri->v[1].position[0] * tri->lambda[1] + \
		tri->v[2].position[0] * tri->lambda[2];
	pPosition->y = \
		tri->v[0].position[1] * tri->lambda[0] + \
		tri->v[1].position[1] * tri->lambda[1] + \
		tri->v[2].position[1] * tri->lambda[2];

	// If I am not mistaken, this is linear in screen space too
	pPosition->z = \
		tri->v[0].position[2] * tri->lambda[0] + \
		tri->v[1].position[2] * tri->lambda[1] + \
		tri->v[2].position[2] * tri->lambda[2];

	if (perspective)
        pPosition->w = 1 / (
			tri->invW[0] * tri->lambda[0] + \
			tri->invW[1] * tri->lambda[1] + \
			tri->invW[2] * tri->lambda[2]
		);
	else  // affine
        pPosition->w = 1.;
}

static void triangleInterpolateAttributes(
	SRPTriangle* tri, const SRPShaderProgram* restrict sp,
	vec4d* pPosition, SRPInterpolated* pInterpolatedBuffer
)
{
	const bool perspective = srpContext.interpolationMode == SRP_INTERPOLATION_MODE_PERSPECTIVE;
	interpolateAttributes(tri->v, 3, tri->lambda, tri->invW, pPosition->w, perspective, sp, pInterpolatedBuffer);
}
