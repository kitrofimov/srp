// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "color.h"
#include "vec_p.h"
#include "context.h"
#include "triangle.h"
#include "framebuffer_p.h"
#include "shaders.h"
#include "fixed_point.h"
#include "math_utils.h"

/** @file
 *  Triangle rasteization & data interpolation */

/** @ingroup Rasterization
 *  @{ */

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

/** Calculate the determinant values for a given point
 *  Determinant values are needed to determine the position of a point in
 *  comparison with edge vector: it is zero on the edge, negative on one side
 *  and positive on the other.
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
	const fixed_24_8 bias[3], vec2fix_24_8 point, fixed_24_8 determinants[3]
);

/** Calculate the delta values for determinants
 *  Those answer the question "how much does the determinants change if a 1 is added to x/y?"
 *  @param[in] edges A pointer to 3-element array containing fixed-point edge
 *                   vectors, as returned from calculateFixedPointEdges()
 *  @param[in] d_dx A pointer to 3-element array that will contain the delta X
 *                  value for determinants
 *  @param[in] d_dx A pointer to 3-element array that will contain the delta Y
 *                  value for determinants */
static void calculateDeterminantDeltas
	(const vec2fix_24_8 edges[3], fixed_24_8 d_dx[3], fixed_24_8 d_dy[3]);

/** @} */  // ingroup Rasterization

void drawTriangle(
	const SRPFramebuffer* fb, const SRPvsOutput vertices[3],
	const SRPShaderProgram* restrict sp, size_t primitiveID
)
{
	/** All calculations related to inside/outside test are done in fixed-point
	 *  24-8 format to achieve more *calculation* precision as compared to
	 *  floating point (if used floating point, there would take place gaps
	 *  between triangles which would be a result of *calculation* precision
	 *  errors)
	 *
	 *  The algorithm used is described by Juan Pineda in his paper called
	 *  "A Parallel Algorithm for Polygon Rasterization", 1988 (see the "see
	 *  "also" section)
	 *  @see https://www.cs.drexel.edu/~deb39/Classes/Papers/comp175-06-pineda.pdf
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

	fixed_24_8 bias[3];             // Determinants' biases: needed for top-left fill convention
	fixed_24_8 wLeft[3];            // Determinants at the start of each line
	fixed_24_8 dw_dx[3], dw_dy[3];  // How much does determinants change if 1 is added to x (y)?
	calculateDeterminantBiases(edges, bias);
	calculateDeterminantsForPoint(positions, edges, bias, min, wLeft);
	calculateDeterminantDeltas(edges, dw_dx, dw_dy);

	// Barycentric coordinates at the start point
	const double triangleAreaX2 = FIXED_24_8_TO_DOUBLE(abs(calculateDeterminant(edges[0], edges[2])));
	const double bStart[3] = {
		FIXED_24_8_TO_DOUBLE(wLeft[0]) / triangleAreaX2,
		FIXED_24_8_TO_DOUBLE(wLeft[1]) / triangleAreaX2,
		FIXED_24_8_TO_DOUBLE(wLeft[2]) / triangleAreaX2
	};

	// Debug/test code
	double aVertex[3] = {  // Attribute value of each vertex
		*(double*) vertices[0].pOutputVariables,
		*(double*) vertices[1].pOutputVariables,
		*(double*) vertices[2].pOutputVariables
	};
	// Attribute value at the start of each line
	double aLeft = bStart[0] * aVertex[0] + bStart[1] * aVertex[1] + bStart[2] * aVertex[2];
	
	// GOOD UNTIL HERE

	double da_dx = \
		FIXED_24_8_TO_DOUBLE(dw_dx[0]) / triangleAreaX2 * aVertex[0] + \
		FIXED_24_8_TO_DOUBLE(dw_dx[1]) / triangleAreaX2 * aVertex[1] + \
		FIXED_24_8_TO_DOUBLE(dw_dx[2]) / triangleAreaX2 * aVertex[2];
	double da_dy = \
		FIXED_24_8_TO_DOUBLE(dw_dy[0]) / triangleAreaX2 * aVertex[0] + \
		FIXED_24_8_TO_DOUBLE(dw_dy[1]) / triangleAreaX2 * aVertex[1] + \
		FIXED_24_8_TO_DOUBLE(dw_dy[2]) / triangleAreaX2 * aVertex[2];

	for (fixed_24_8 y = min.y; y <= max.y; y += FIXED_24_8_ONE)
	{
		// Determinants for current pixel
		fixed_24_8 w[3] = {wLeft[0], wLeft[1], wLeft[2]};
		// Attribute for current pixel
		double a = aLeft;

		for (fixed_24_8 x = min.x; x <= max.x; x += FIXED_24_8_ONE)
		{
			if ((w[0] | w[1] | w[2]) >= 0)
			{
				SRPColor color = {a * 255., 0, 0, 255}; 
				srpFramebufferDrawPixel(
					fb, FIXED_24_8_TO_INT(FIXED_24_8_FLOOR(x)),
					FIXED_24_8_TO_INT(FIXED_24_8_FLOOR(y)), 0,
					SRP_COLOR_TO_UINT32_T(color)
				);
			}

			w[0] += dw_dx[0];
			w[1] += dw_dx[1];
			w[2] += dw_dx[2];

			a += da_dx;
		}
		wLeft[0] += dw_dy[0];
		wLeft[1] += dw_dy[1];
		wLeft[2] += dw_dy[2];

		aLeft += da_dy;
	}
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
	const fixed_24_8 bias[3], vec2fix_24_8 point, fixed_24_8 determinants[3]
)
{
	// If we bias a determinant once, there is no need to "rebias" it again if
	// it is computed incrementally
	determinants[0] = \
		calculateDeterminant(vec2fp_24_8_subtract(point, positions[1]), edges[1]) - bias[1];
	determinants[1] = \
		calculateDeterminant(vec2fp_24_8_subtract(point, positions[2]), edges[2]) - bias[2];
	determinants[2] = \
		calculateDeterminant(vec2fp_24_8_subtract(point, positions[0]), edges[0]) - bias[0];
}

static void calculateDeterminantDeltas
	(const vec2fix_24_8 edges[3], fixed_24_8 d_dx[3], fixed_24_8 d_dy[3])
{
	d_dx[0] =  edges[1].y;
	d_dx[1] =  edges[2].y;
	d_dx[2] =  edges[0].y;

	d_dy[0] = -edges[1].x;
	d_dy[1] = -edges[2].x;
	d_dy[2] = -edges[0].x;
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

