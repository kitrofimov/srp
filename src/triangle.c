// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "triangle.h"
#include "context.h"
#include "vertex.h"
#include "framebuffer_p.h"
#include "message_callback_p.h"
#include "shaders.h"
#include "color.h"
#include "utils.h"
#include "math_utils.h"
#include "vec.h"
#include "fragment.h"

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

bool setupTriangle(
	SRPTriangle* tri, const SRPFramebuffer* fb
)
{
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
	/** Let @f$ v_0, v_1, v_2 @f$ be points in space that form a triangle and
	 *  @f$ a, b, c @f$ be barycentric coordinates for a point @f$ P @f$
	 *  according to @f$ v_0, v_1, v_2 @f$ respectively. Then, by the property
	 *  of barycentric coordinates @f$ P = av_0 + bv_1 + cv_2 @f$. This can be
	 *  extrapolated to arbitrary values assigned to vertices, and this is called
	 *  affine attribute interpolation.
     *
	 *  But this method does not take the perspective divide into account, so
	 *  the texture (for example) will look "wrong".
	 *
	 *  It can be shown (see the "see also" section) that perspective-correct Z
	 *  value can be obtained by taking the reciprocal of the linear interpolation
	 *  between the reciprocals of input Z values, and similarly for arbitrary
	 *  parameters.
	 *
	 *  @see https://www.comp.nus.edu.sg/%7Elowkl/publications/lowk_persp_interp_techrep.pdf
	 *  @see https://www.youtube.com/watch?v=F5X6S35SW2s */

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

	// If I am not mistaken, this should be linear in screen space
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
	// vertices[i].pOutputVariables =
	// (                        Vi                              )
	// (          ViA0          )(          ViA1          ) ...
	// (ViA0E0 ViA0E1 ... ViA0En)(ViA1E0 ViA1E1 ... ViA1En) ...
	// [V]ertex, [A]ttribute, [E]lement

	bool perspective = (srpContext.interpolationMode == SRP_INTERPOLATION_MODE_PERSPECTIVE);

	// Points to current attribute in output buffer
	void* pInterpolatedAttrVoid = pInterpolatedBuffer;

	size_t attrOffsetBytes = 0;
	for (size_t attrI = 0; attrI < sp->vs->nOutputVariables; attrI++)
	{
		SRPVertexVariableInformation* attr = &sp->vs->outputVariablesInfo[attrI];
		size_t elemSize = 0;
		switch (attr->type)
		{
		case TYPE_DOUBLE:
		{
			elemSize = sizeof(double);
			double* pInterpolatedAttr = (double*) pInterpolatedAttrVoid;

			// Pointers to the current attribute of 0th, 1st and 2nd vertices
			double* AV[3];
			for (int i = 0; i < 3; i++)
				AV[i] = (double*) ADD_VOID_PTR(tri->v[i].pOutputVariables, attrOffsetBytes);

			if (perspective)
				for (size_t elemI = 0; elemI < attr->nItems; elemI++)
				{
					pInterpolatedAttr[elemI] = pPosition->w * (
						AV[0][elemI] * tri->invW[0] * tri->lambda[0] + \
						AV[1][elemI] * tri->invW[1] * tri->lambda[1] + \
						AV[2][elemI] * tri->invW[2] * tri->lambda[2]
					);
				}
			else  // affine
				for (size_t elemI = 0; elemI < attr->nItems; elemI++)
				{
					pInterpolatedAttr[elemI] = \
						AV[0][elemI] * tri->lambda[0] + \
						AV[1][elemI] * tri->lambda[1] + \
						AV[2][elemI] * tri->lambda[2];
				}
			break;
		}
		default:
			srpMessageCallbackHelper(
				SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
				"Unexpected type (%i)", attr->type
			);
		}

		size_t attrSize = elemSize * attr->nItems;
		pInterpolatedAttrVoid = ADD_VOID_PTR(pInterpolatedAttrVoid, attrSize);
		attrOffsetBytes += attrSize;
	}
}
