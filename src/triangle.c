// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "color.h"
#include "message_callback_p.h"
#include "type.h"
#include "utils.h"
#include "vec_p.h"
#include "context.h"
#include "triangle.h"
#include "framebuffer_p.h"
#include "shaders.h"
#include "fixed_point.h"
#include "math_utils.h"
#include "vertex.h"

/** @file
 *  Triangle rasteization & data interpolation */

/** @ingroup Rasterization
 *  @{ */

/** Static; represents the determinant value in triangle draw call
 *  Uses SoA (https://en.wikipedia.org/wiki/AoS_and_SoA) to make use of SIMD
 *  @see drawTriangle(), Barycentrics */
typedef struct Determinants
{
	fixed_24_8 value[3];  /**< Current values */
	fixed_24_8 left [3];  /**< Values at the start of the current scanline */
	fixed_24_8 d_dx [3];  /**< How much does the values change if 1 is added to X */
	fixed_24_8 d_dy [3];  /**< How much does the values change if 1 is added to Y */
} Determinants;

/** Static; represents the barycentric coordinate value in triangle draw call
 *  Uses SoA (https://en.wikipedia.org/wiki/AoS_and_SoA) to make use of SIMD
 *  @see drawTriangle(), Determinants */
typedef struct Barycentrics
{
	double value[3];  /**< Current values */
	double left [3];  /**< Values at the start of the current scanline */
	double d_dx [3];  /**< How much does the values change if 1 is added to X */
	double d_dy [3];  /**< How much does the values change if 1 is added to Y */
} Barycentrics;

/** Calculate the determinant value (effectively a signed area of a
 *  parallelogram formed by two vectors) for a point and an edge using an edge
 *  vector (A->B) and a vertex-to-point vector (A->P).
 *  The determinant value determines where the point P lies with respect to the
 *  edge vector: 0 => P lies on the edge; >0 => P lies "where we need it", that
 *  is, inside the triangle's halfspace; <0 => P lies "where we do not need in",
 *  that is, outside the triangle's halfspace
 *  @param[in] ab An edge vector (A->B)
 *  @param[in] ap A vertex-to-point vector (A->P)
 *  @retutn The determinant value */
static fixed_24_8 calculateDeterminant(vec2fix_24_8 ab, vec2fix_24_8 ap);

/** Check if a triangle's edge is flat top or left. Used in rasterization rules.
 *  @param[in] edge A pointer to an edge vector (pointing from one vertex to the
 *  other, assuming counter-clockwise vertex ordering)
 *  @return Whether or not this edge is flat top or left */
static bool isEdgeFlatTopOrLeft(vec2fix_24_8 edge);

/** Convert the NDC vertex positions stored in `vertices` to fixed-point 24-8
 *  representation
 *  @param[in] fb A pointer to the framebuffer used to draw these vertices
 *  @param[in] vertices A pointer to 3-element array of SRPvsOutput,
 *                      as returned from vertex shader
 *  @param[out] positions A pointer to 3-element array of vec2fix_24_8 that will
 *                        contain the positions after this call */
static void calculateFixedPointPositions
	(const SRPFramebuffer* fb, const SRPvsOutput vertices[3], vec2fix_24_8 positions[3]);

/** Calculate the fixed-point edge vectors from fixed-point vertex positions
 *  @param[in] positions A pointer to 3-element array of fixed-point vertex
 *                       positions, as returned from calculateFixedPointPositions()
 *  @param[out] edges A pointer to 3-element array of vec2fix_24_8 that will
 *                    contain the edge vectors after this call */
static void calculateFixedPointEdges(const vec2fix_24_8 positions[3], vec2fix_24_8 edges[3]);

/** Whether or not to cull this triangle.
 *  Uses state from SRPContext (SRPContext.cullFace, SRPContext.frontFace) to
 *  determine whether or not to cull this triangle.
 *  @param[in] edges A pointer to 3-element array containing fixed-point edge
 *                   vectors, as returned from calculateFixedPointEdges()
 *  @return `true` if the triangle should be culled, `false` otherwise */
static bool cullTriangle(const vec2fix_24_8 edges[3]);

/** Get the two poins defining the triangle's bounding box
 *  @param[in] positions A pointer to 3-element array of fixed-point vertex
 *                       positions, as returned from calculateFixedPointPositions()
 *  @param[out] min The first point
 *  @param[out] max The second point */
static void calculateBoundingBox
	(const vec2fix_24_8 positions[3], vec2fix_24_8* min, vec2fix_24_8* max);

/** Calculate the bias values for the determinants.
 *  This is needed to follow the fill convention: we should "push" points that
 *  lie on some edges (defined by fill convention) towards the triangle so no
 *  overdraw occurs. This "push" is effectively just subtracting some value from
 *  determinant. This subtracted value is called a bias value.
 *  @param[in] edges A pointer to 3-element array containing fixed-point edge
 *                   vectors, as returned from calculateFixedPointEdges()
 *  @paran[out] bias A pointer to 3-element array that will contain the bias
 *                   values after this call */
static void calculateDeterminantBiases(const vec2fix_24_8 edges[3], fixed_24_8 bias[3]);

/*  Calculate the determinant values for a given point
 *  Determinant values are needed to determine the position of a point in
 *  comparison with edge vector: it is zero on the edge, negative on one side
 *  and positive on the other.
 *  @todo update docs
 *  @param[in] positions A pointer to 3-element array of fixed-point vertex
 *                       positions, as returned from calculateFixedPointPositions()
 *  @param[in] edges A pointer to 3-element array containing fixed-point edge
 *                   vectors, as returned from calculateFixedPointEdges()
 *  @param[in] bias A pointer to 3-element array comtaining bias values, as
 *                  returned from calculateDeterminantBiases()
 *  @param[in] point A point for which to compute the determinants
 *  @param[out] determinants A pointer to 3-element array that will contain the
 *                           determinants after this call */
static void calculateDeterminantsForPoint(
	const vec2fix_24_8 positions[3], const vec2fix_24_8 edges[3],
	vec2fix_24_8 point, Determinants* d
);

static void determinantsStep(fixed_24_8 values[3], const fixed_24_8 deltas[3]);

static double calculateTriangleAreaX2(const vec2fix_24_8 edges[3]);

static void calculateBarycentricCoordinates
	(const Determinants* d, const vec2fix_24_8 edges[3], Barycentrics* b);

static void interpolationInitialize(
	const SRPvsOutput vertices[3], const SRPShaderProgram* sp, const Barycentrics* b,
	void* iVarBuffer, void* d_iv_dx_buffer, void* d_iv_dy_buffer
);

static void interpolationStep
	(const SRPShaderProgram* sp, void* iVarBuffer, const void* d_iv_dx_buffer);

/** @} */  // ingroup Rasterization

void drawTriangle(
	const SRPFramebuffer* fb, const SRPvsOutput vertices[3],
	const SRPShaderProgram* restrict sp, size_t primitiveID
)
{
	/** The algorithm used is described by Juan Pineda in his paper called
	 *  "A Parallel Algorithm for Polygon Rasterization", 1988 (see the "see
	 *  "also" section)
	 *  @see https://www.cs.drexel.edu/~deb39/Classes/Papers/comp175-06-pineda.pdf
	 *
	 *  All calculations related to inside/outside test are done in fixed-point
	 *  24-8, but all calculations related to data interpolation (barycentric
	 *  coordinates & etc.) are performed in double format.
	 *
	 *  Why not to use barycentric coordinates both for interpolation and
	 *  inside-outside tests? The answer is simple: inside-outside tests
	 *  require precision in calculations (solution: fixed-point arithmetic),
	 *  while interpolation (and barycentric coordiantes) in precise require
	 *  numerical precision for values near zero (solution: floating point OR
	 *  fixed-point with A LOT of bits for fractional part; but I do not want
	 *  to mess with arbitrary-precision integers to handle these many bits)
	 *
	 *  Read the documentation for functions called by this one to get more
	 *  information about the implementation */

	vec2fix_24_8 positions[3], edges[3];
	calculateFixedPointPositions(fb, vertices, positions);
	calculateFixedPointEdges(positions, edges);

	if (cullTriangle(edges))
		return;

	vec2fix_24_8 min, max;
	calculateBoundingBox(positions, &min, &max);

	Determinants d;
	Barycentrics b;
	calculateDeterminantsForPoint(positions, edges, min, &d);
	calculateBarycentricCoordinates(&d, edges, &b);

	uint8_t iVariablesLeft[sp->vs->nBytesPerOutputVariables];
	uint8_t d_iv_dx[sp->vs->nBytesPerOutputVariables];
	uint8_t d_iv_dy[sp->vs->nBytesPerOutputVariables];
	interpolationInitialize(vertices, sp, &b, iVariablesLeft, d_iv_dx, d_iv_dy);

	for (fixed_24_8 y = min.y; y <= max.y; y += FIXED_24_8_ONE)
	{
		memcpy(d.value, d.left, sizeof(d.left));

		// Interpolated variables for current pixel
		uint8_t iVariables[sp->vs->nBytesPerOutputVariables];
		memcpy(iVariables, iVariablesLeft, sp->vs->nBytesPerOutputVariables);

		for (fixed_24_8 x = min.x; x <= max.x; x += FIXED_24_8_ONE)
		{
			if ((d.value[0] | d.value[1] | d.value[2]) >= 0)
			{
				SRPfsInput fsIn = {
					.uniform = sp->uniform,
					.interpolated = (SRPInterpolated*) iVariables,
					.fragCoord = {0, 0, 0, 0},
					.frontFacing = true,
					.primitiveID = primitiveID
				};
				SRPfsOutput fsOut = {0};

				sp->fs->shader(&fsIn, &fsOut);

				SRPColor color = {
					fsOut.color[0] * 255,
					fsOut.color[1] * 255,
					fsOut.color[2] * 255,
					fsOut.color[3] * 255
				};

				srpFramebufferDrawPixel(
					fb, FIXED_24_8_TO_INT(FIXED_24_8_FLOOR(x)),
					FIXED_24_8_TO_INT(FIXED_24_8_FLOOR(y)), 0,
					SRP_COLOR_TO_UINT32_T(color)
				);
			}

			determinantsStep(d.value, d.d_dx);
			interpolationStep(sp, iVariables, d_iv_dx);
		}  // X traversal

		determinantsStep(d.left, d.d_dy);
		interpolationStep(sp, iVariablesLeft, d_iv_dy);
	}  // Y traversal
}

static void calculateFixedPointPositions
	(const SRPFramebuffer* fb, const SRPvsOutput vertices[3], vec2fix_24_8 positions[3])
{
	for (uint8_t i = 0; i < 3; i++)
	{
		double ndc[2];
		srpFramebufferNDCToScreenSpace(fb, vertices[i].position, ndc);
		positions[i] = (vec2fix_24_8) {
			FIXED_24_8_FROM_DOUBLE(ndc[0]),
			FIXED_24_8_FROM_DOUBLE(ndc[1])
		};
	}
}

static void calculateFixedPointEdges(const vec2fix_24_8 positions[3], vec2fix_24_8 edges[3])
{
	for (uint8_t i = 0; i < 3; i++)
		edges[i] = vec2fp_24_8_subtract(positions[(i+1)%3], positions[i]);
}

static bool cullTriangle(const vec2fix_24_8 edges[3])
{
	// Cross(e0, e2) == -Cross(e0, e2) == -Cross(v0->v1, v0->v2)
	fixed_24_8 orientation = -calculateDeterminant(edges[0], edges[2]);
	bool clockwise = orientation > 0;

	if (orientation == 0)  // degenerate
		return true;
	else if (srpContext.cullFace == SRP_CULL_FACE_FRONT_AND_BACK)
		return true;
	else if (srpContext.frontFace == SRP_FRONT_FACE_CLOCKWISE)
	{
		bool cullFrontAndCW = srpContext.cullFace == SRP_CULL_FACE_FRONT && clockwise;
		bool cullBackAndCWW = srpContext.cullFace == SRP_CULL_FACE_BACK && !clockwise;
		if (cullFrontAndCW || cullBackAndCWW)
			return true;
	}
	else  // counter-clockwise is considered front face
	{
		bool cullFrontAndCCW = srpContext.cullFace == SRP_CULL_FACE_FRONT && !clockwise;
		bool cullBackAndCW = srpContext.cullFace == SRP_CULL_FACE_BACK && clockwise;
		if (cullFrontAndCCW || cullBackAndCW)
			return true;
	}
	return false;
}

static void calculateBoundingBox
	(const vec2fix_24_8 positions[3], vec2fix_24_8* min, vec2fix_24_8* max)
{
	// Flooring and adding 0.5 to loop over CENTERS of pixels
	fixed_24_8 x_min = \
		FIXED_24_8_FLOOR(MIN(positions[0].x, MIN(positions[1].x, positions[2].x))) + \
		FIXED_24_8_FROM_DOUBLE(0.5);
	fixed_24_8 y_min = \
		FIXED_24_8_FLOOR(MIN(positions[0].y, MIN(positions[1].y, positions[2].y))) + \
		FIXED_24_8_FROM_DOUBLE(0.5);
	fixed_24_8 x_max = \
		FIXED_24_8_FLOOR(MAX(positions[0].x, MAX(positions[1].x, positions[2].x))) + \
		FIXED_24_8_FROM_DOUBLE(0.5);
	fixed_24_8 y_max = \
		FIXED_24_8_FLOOR(MAX(positions[0].y, MAX(positions[1].y, positions[2].y))) + \
		FIXED_24_8_FROM_DOUBLE(0.5);

	*min = (vec2fix_24_8) {x_min, y_min};
	*max = (vec2fix_24_8) {x_max, y_max};
}

static void calculateDeterminantBiases(const vec2fix_24_8 edges[3], fixed_24_8 bias[3])
{
	bias[0] = isEdgeFlatTopOrLeft(edges[0]) ? FIXED_24_8_MIN_VALUE : 0;
	bias[1] = isEdgeFlatTopOrLeft(edges[1]) ? FIXED_24_8_MIN_VALUE : 0;
	bias[2] = isEdgeFlatTopOrLeft(edges[2]) ? FIXED_24_8_MIN_VALUE : 0;
}

static void calculateDeterminantsForPoint(
	const vec2fix_24_8 positions[3], const vec2fix_24_8 edges[3],
	vec2fix_24_8 point, Determinants* det
)
{
	// Calculate values
	fixed_24_8 bias[3];  // Needed for fill convention
	calculateDeterminantBiases(edges, bias);

	// If we bias a determinant once, there is no need to "rebias" it again if
	// it is computed incrementally
	det->left[0] = \
		calculateDeterminant(vec2fp_24_8_subtract(point, positions[1]), edges[1]) - bias[1];
	det->left[1] = \
		calculateDeterminant(vec2fp_24_8_subtract(point, positions[2]), edges[2]) - bias[2];
	det->left[2] = \
		calculateDeterminant(vec2fp_24_8_subtract(point, positions[0]), edges[0]) - bias[0];

	// Calculate deltas
	det->d_dx[0] =  edges[1].y;
	det->d_dx[1] =  edges[2].y;
	det->d_dx[2] =  edges[0].y;
	det->d_dy[0] = -edges[1].x;
	det->d_dy[1] = -edges[2].x;
	det->d_dy[2] = -edges[0].x;
}

static void determinantsStep(fixed_24_8 values[3], const fixed_24_8 deltas[3])
{
	values[0] += deltas[0];
	values[1] += deltas[1];
	values[2] += deltas[2];
}

static double calculateTriangleAreaX2(const vec2fix_24_8 edges[3])
{
	return abs(calculateDeterminant(edges[0], edges[2]));
}

static void calculateBarycentricCoordinates
	(const Determinants* det, const vec2fix_24_8 edges[3], Barycentrics* bar)
{
	// Calculate values
	const double triangleAreaX2 = FIXED_24_8_TO_DOUBLE(calculateTriangleAreaX2(edges));
	bar->left[0] = FIXED_24_8_TO_DOUBLE(det->left[0]) / triangleAreaX2;
	bar->left[1] = FIXED_24_8_TO_DOUBLE(det->left[1]) / triangleAreaX2;
	bar->left[2] = FIXED_24_8_TO_DOUBLE(det->left[2]) / triangleAreaX2;

	// Calculate deltas
	bar->d_dx[0] = FIXED_24_8_TO_DOUBLE(det->d_dx[0]) / triangleAreaX2;
	bar->d_dx[1] = FIXED_24_8_TO_DOUBLE(det->d_dx[1]) / triangleAreaX2;
	bar->d_dx[2] = FIXED_24_8_TO_DOUBLE(det->d_dx[2]) / triangleAreaX2;
	bar->d_dy[0] = FIXED_24_8_TO_DOUBLE(det->d_dy[0]) / triangleAreaX2;
	bar->d_dy[1] = FIXED_24_8_TO_DOUBLE(det->d_dy[1]) / triangleAreaX2;
	bar->d_dy[2] = FIXED_24_8_TO_DOUBLE(det->d_dy[2]) / triangleAreaX2;
}

static void interpolationInitialize(
	const SRPvsOutput vertices[3], const SRPShaderProgram* sp, const Barycentrics* b,
	void* iVarBuffer, void* d_iv_dx_buffer, void* d_iv_dy_buffer
)
{
	// Postfixed with `_void` to indicate that these do not have type information
	// (as opposed to variables without `_void` postfix)

	// Pointer to the value of current variable of 0th, 1st and 2nd vertices
	const void* var_0_void = vertices[0].pOutputVariables;
	const void* var_1_void = vertices[1].pOutputVariables;
	const void* var_2_void = vertices[2].pOutputVariables;
	// Pointer to the interpolated value of current variable
	void* i_var_void = iVarBuffer;
	// How much does the interpolated value of current variable change,
	// if 1 is added to x (or y)?
	void* d_iv_dx_void = d_iv_dx_buffer;
	void* d_iv_dy_void = d_iv_dy_buffer;

	for (size_t varI = 0; varI < sp->vs->nOutputVariables; varI++)
	{
		SRPVertexVariableInformation* info = &sp->vs->outputVariablesInfo[varI];
		size_t elemSize = 0;

		switch (info->type)
		{
		case TYPE_DOUBLE:
		{
			elemSize = sizeof(double);

			// Assign type information to pointers we work with
			const double* var_0 = var_0_void;
			const double* var_1 = var_1_void;
			const double* var_2 = var_2_void;
			double* i_var   = i_var_void;
			double* d_iv_dx = d_iv_dx_void;
			double* d_iv_dy = d_iv_dy_void;

			// Calculate the interpolated value and deltas for every element
			// of current variable
			for (size_t elemI = 0; elemI < info->nItems; elemI++)
			{
				i_var[elemI] = (
					var_0[elemI] * b->left[0] + \
					var_1[elemI] * b->left[1] + \
					var_2[elemI] * b->left[2]
				);
				d_iv_dx[elemI] = (
					var_0[elemI] * b->d_dx[0] + \
					var_1[elemI] * b->d_dx[1] + \
					var_2[elemI] * b->d_dx[2]
				);
				d_iv_dy[elemI] = (
					var_0[elemI] * b->d_dy[0] + \
					var_1[elemI] * b->d_dy[1] + \
					var_2[elemI] * b->d_dy[2]
				);
			}

			break;
		}
		default:
			srpMessageCallbackHelper(
				SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
				"Unexpected type (%i)\n", info->type
			);
			return;
		}

		// Update pointers
		var_0_void   = ADD_VOID_PTR(  var_0_void, info->nItems * elemSize);
		var_1_void   = ADD_VOID_PTR(  var_1_void, info->nItems * elemSize);
		var_2_void   = ADD_VOID_PTR(  var_2_void, info->nItems * elemSize);
		i_var_void   = ADD_VOID_PTR(  i_var_void, info->nItems * elemSize);
		d_iv_dx_void = ADD_VOID_PTR(d_iv_dx_void, info->nItems * elemSize);
		d_iv_dy_void = ADD_VOID_PTR(d_iv_dy_void, info->nItems * elemSize);
	}
}

static void interpolationStep
	(const SRPShaderProgram* sp, void* iVarBuffer, const void* d_iv_buffer)
{
	// Postfixed with `_void` to indicate that these do not have type information
	// (as opposed to variables without `_void` postfix)

	// Pointer to the interpolated value of current variable
	void* i_var_void = iVarBuffer;
	// Pointer to the current interpolated variable's derivative
	const void* d_iv_void = d_iv_buffer;

	for (size_t varI = 0; varI < sp->vs->nOutputVariables; varI++)
	{
		SRPVertexVariableInformation* info = &sp->vs->outputVariablesInfo[varI];
		size_t elemSize = 0;

		switch (info->type)
		{
		case TYPE_DOUBLE:
		{
			elemSize = sizeof(double);

			// Assign type information to pointers we work with
			double* i_var   = i_var_void;
			const double* d_iv = d_iv_void;

			// Calculate the interpolated value and deltas for every element
			// of current variable
			for (size_t elemI = 0; elemI < info->nItems; elemI++)
				i_var[elemI] += d_iv[elemI];

			break;
		}
		default:
			srpMessageCallbackHelper(
				SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
				"Unexpected type (%i)\n", info->type
			);
			return;
		}

		// Update pointers
		i_var_void = ADD_VOID_PTR(i_var_void, info->nItems * elemSize);
		d_iv_void  = ADD_VOID_PTR( d_iv_void, info->nItems * elemSize);
	}
}

static fixed_24_8 calculateDeterminant(vec2fix_24_8 ab, vec2fix_24_8 ap)
{
	return FIXED_24_8_MULTIPLY(ab.x, ap.y) - FIXED_24_8_MULTIPLY(ab.y, ap.x);
}

static bool isEdgeFlatTopOrLeft(vec2fix_24_8 edge)
{
	bool flatTop = edge.x < 0 && edge.y == 0;
	bool left = edge.y > 0;
	return flatTop || left;
}

