// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <stdint.h>
#include <stdio.h>
#include "vec_p.h"
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

/** @} */  // ingroup Rasterization

void drawTriangle(
	const SRPFramebuffer* fb, const SRPvsOutput vertices[3],
	const SRPShaderProgram* restrict sp, size_t primitiveID
)
{
	vec2fix_24_8 positions[3], edges[3];
	// Cannot "unify" these loops since edges computation will touch uninitialized memory
	for (uint8_t i = 0; i < 3; i++)
	{
		double ndc[2];
		srpFramebufferNDCToScreenSpace(fb, vertices[i].position, ndc);
		positions[i] = (vec2fix_24_8) {
			FIXED_24_8_FROM_DOUBLE(ndc[0]),
			FIXED_24_8_FROM_DOUBLE(ndc[1])
		};
	}
	for (uint8_t i = 0; i < 3; i++)
		edges[i] = vec2fp_24_8_subtract(positions[(i+1)%3], positions[i]);

	// Points that define the bounding box
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

	// Starting point for traversal
	vec2fix_24_8 min = {x_min, y_min};

	// Bias values for each edge: needed for top-left rasterization rule
	fixed_24_8 bias[3] = {
		isEdgeFlatTopOrLeft(edges[0]) ? FIXED_24_8_MIN_VALUE : 0,
		isEdgeFlatTopOrLeft(edges[1]) ? FIXED_24_8_MIN_VALUE : 0,
		isEdgeFlatTopOrLeft(edges[2]) ? FIXED_24_8_MIN_VALUE : 0
	};

	// !!! incorrect comment
	// Barycentric coordinates for each vertex at the start of the current line
	// (that is, for vertex 0, the ratio S{Pv1v2} / S{v0v1v2}, for vertex 1 =
	// S{Pv0v2} / S{v0v1v2}); so w[0] tells the position of P with respect to
	// edges[1], w[1] - edges[2] and w[2] - edges[0]
	fixed_24_8 wLeft[3] = {
		calculateDeterminant(edges[1], vec2fp_24_8_subtract(min, positions[1])) - bias[1],
		calculateDeterminant(edges[2], vec2fp_24_8_subtract(min, positions[2])) - bias[2],
		calculateDeterminant(edges[0], vec2fp_24_8_subtract(min, positions[0])) - bias[0],
	};

	fixed_24_8 dw_dx[3] = { edges[1].y,  edges[2].y,  edges[0].y};
	fixed_24_8 dw_dy[3] = {-edges[1].x, -edges[2].x, -edges[0].x};

	for (fixed_24_8 y = y_min; y <= y_max; y += FIXED_24_8_ONE)
	{
		// See wLeft
		fixed_24_8 w[3] = {wLeft[0], wLeft[1], wLeft[2]};

		for (fixed_24_8 x = x_min; x <= x_max; x += FIXED_24_8_ONE)
		{
			if ((w[0] | w[1] | w[2]) >= 0)
				srpFramebufferDrawPixel(
					fb, FIXED_24_8_TO_INT(FIXED_24_8_FLOOR(x)),
					FIXED_24_8_TO_INT(FIXED_24_8_FLOOR(y)), 0, 0xFFFFFFFF
				);

			w[0] += dw_dx[0];
			w[1] += dw_dx[1];
			w[2] += dw_dx[2];
		}
		wLeft[0] += dw_dy[0];
		wLeft[1] += dw_dy[1];
		wLeft[2] += dw_dy[2];
	}
}

static fixed_24_8 calculateDeterminant(vec2fix_24_8 ab, vec2fix_24_8 ap)
{
	return FIXED_24_8_MULTIPLY(ab.y, ap.x) - FIXED_24_8_MULTIPLY(ab.x, ap.y);
}

static bool isEdgeFlatTopOrLeft(vec2fix_24_8 edge)
{
	bool flatTop = edge.x < 0 && edge.y == 0;
	bool left = edge.y > 0;
	return flatTop || left;
}

