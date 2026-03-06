// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Rasterization
 *  Triangle rasterization implementation */

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

/** @ingroup Rasterization
 *  @{ */

/** Get a parallelogram's signed area. The two vectors define a parallelogram.
 *  Used for barycentric coordinates' initialization in calculateBarycentrics() */
static float signedAreaParallelogram(
	const vec3* restrict a, const vec3* restrict b
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
static void calculateBarycentrics(SRPTriangle* tri, float areaX2, vec2 point);

/** Check if a triangle's edge is flat top or left. Assumes counter-clockwise
 *  vertex order!
 *  @param[in] edge A pointer to an edge vector (pointing from one vertex
 *  				to the other)
 *  @return Whether or not this edge is flat top or left */
static bool isEdgeFlatTopOrLeft(const vec3* restrict edge);

/** Interpolate the fragment position and vertex variables inside the triangle.
 *  @param[in] tri Triangle to interpolate data for
 *  @param[in] sp A pointer to shader program to use
 *  @param[out] pInterpolatedBuffer A pointer to the buffer where interpolated
 *              variables will appear. Must be big enough to hold all of them
 *  @param[out] depth Fragment depth
 *  @param[out] recIntInvW Reciprocal of interpolated inverse Wclip */
static void triangleInterpolateData(
	SRPTriangle* tri, const SRPShaderProgram* restrict sp,
	SRPInterpolated* pInterpolatedBuffer, float* depth, float* recIntInvW
);

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
				bool inside = (tri->lambda[i] > 0.) || (ROUGHLY_ZERO(tri->lambda[i]) && tri->edgeTL[i]);
				if (!inside)
					goto nextPixel;
			}

			float depth, recIntInvW;
			triangleInterpolateData(tri, sp, interpolatedBuffer, &depth, &recIntInvW);

			SRPFragmentShaderIn fsIn = {
				.uniform = sp->uniform,
				.varyings = interpolatedBuffer,
				.fragCoord = { x + 0.5, y + 0.5, depth, recIntInvW },
				.frontFacing = tri->isFrontFacing,
				.primitiveID = tri->id,
			};
			emitFragment(fb, sp, x, y, &fsIn);

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

	// vec3 is tightly packed, so this is safe
	for (uint8_t i = 0; i < 3; i++)
		tri->p_ndc[i] = (vec3*) tri->v[i].clipPosition;

	bool isCCW;
	if (shouldCullTriangle(tri, &isCCW, &tri->isFrontFacing))
		return false;

	if (!isCCW)
		triangleChangeWinding(tri);

	for (size_t i = 0; i < 3; i++)
		framebufferNDCToScreenSpace(fb, (float*) tri->p_ndc[i], (float*) &tri->ss[i]);

	for (size_t i = 0; i < 3; i++)
		tri->edge[i] = vec3Subtract(tri->ss[(i+1) % 3], tri->ss[i]);

	// Cull degenerate triangles (using SS area!)
	float areaX2 = fabs(signedAreaParallelogram(&tri->edge[0], &tri->edge[2]));
	if (ROUGHLY_ZERO(areaX2))
		return false;

	// FP errors may lead to one of these being -1 => triangle not drawn
	// Hence assuring it's at least 0 OR at most width/height of the framebuffer
	tri->minBP = VEC2(
		MAX(floor(MIN(tri->ss[0].x, MIN(tri->ss[1].x, tri->ss[2].x))), 0),
		MAX(floor(MIN(tri->ss[0].y, MIN(tri->ss[1].y, tri->ss[2].y))), 0)
	);
	tri->maxBP = VEC2(
		MIN(ceil(MAX(tri->ss[0].x, MAX(tri->ss[1].x, tri->ss[2].x))), fb->width),
		MIN(ceil(MAX(tri->ss[0].y, MAX(tri->ss[1].y, tri->ss[2].y))), fb->height)
	);

	calculateBarycentrics(tri, areaX2, VEC2(tri->minBP.x + 0.5, tri->minBP.y + 0.5));

	for (uint8_t i = 0; i < 3; i++)
	{
		tri->lambda_row[i] = tri->lambda[i];
		tri->edgeTL[i] = isEdgeFlatTopOrLeft(&tri->edge[i]);
	}

	return true;
}

static bool shouldCullTriangle(const SRPTriangle* tri, bool* isCCW, bool* isFrontFacing)
{
	vec3 edge0 = vec3Subtract(*tri->p_ndc[1], *tri->p_ndc[0]);
	vec3 edge1 = vec3Subtract(*tri->p_ndc[2], *tri->p_ndc[0]);
	float signedArea = signedAreaParallelogram(&edge0, &edge1);
	*isCCW = signedArea > 0;

	// Should have already been handled, but just in case
	if (srpContext.raster.cullFace == SRP_FACE_FRONT_AND_BACK)
		return true;

	bool frontFacing = \
		(signedArea > 0) & (srpContext.raster.frontFace == SRP_WINDING_CCW) ||
		(signedArea < 0) & (srpContext.raster.frontFace == SRP_WINDING_CW);
	bool cull = \
		( frontFacing && srpContext.raster.cullFace == SRP_FACE_FRONT) ||
		(!frontFacing && srpContext.raster.cullFace == SRP_FACE_BACK);

	*isFrontFacing = frontFacing;
	return cull;
}

static void triangleChangeWinding(SRPTriangle* tri)
{
	// Not swapping `p_ndc`, because those are pointers to `v.position`
	// If swapped both at the same time, it's the same as not swapping anything
	SRPVertexShaderOut temp1 = tri->v[1];
	tri->v[1] = tri->v[2];
	tri->v[2] = temp1;

	float temp2 = tri->invW[1];
	tri->invW[1] = tri->invW[2];
	tri->invW[2] = temp2;
}

static void calculateBarycentrics(SRPTriangle* tri, float areaX2, vec2 point)
{
	vec3 AP = VEC3(
		point.x - tri->ss[0].x,
		point.y - tri->ss[0].y,
		0
	);
	vec3 BP = VEC3(
		point.x - tri->ss[1].x,
		point.y - tri->ss[1].y,
		0
	);
	vec3 CP = VEC3(
		point.x - tri->ss[2].x,
		point.y - tri->ss[2].y,
		0
	);

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

static float signedAreaParallelogram(
	const vec3* restrict a, const vec3* restrict b
)
{
	return a->x * b->y - a->y * b->x;
}

static bool isEdgeFlatTopOrLeft(const vec3* restrict edge)
{
	return ((edge->x > 0) && ROUGHLY_ZERO(edge->y)) || (edge->y < 0);
}

static void triangleInterpolateData(
	SRPTriangle* tri, const SRPShaderProgram* restrict sp,
	SRPInterpolated* pInterpolatedBuffer, float* depth, float* recIntInvW
)
{
	interpolateDepthAndWTriangle(tri->v, tri->lambda, tri->invW, sp, depth, recIntInvW);
	interpolateAttributes(tri->v, 3, tri->lambda, tri->invW, *recIntInvW, sp, pInterpolatedBuffer);
}

/** @} */  // ingroup Rasterization
