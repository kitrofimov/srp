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

/** Calculate the determinant value for a point and an edge using an edge
 *  vector (A->B) and a vertex-to-point vector (A->P).
 *  The determinant value determines where the point P lies with respect to the
 *  edge vector: 0 => P lies on the edge; >0 => P lies "where we need it", that
 *  is, inside the triangle's halfspace; <0 => P lies "where we do not need in",
 *  that is, outside the triangle's halfspace
 *  @param[in] ab An edge vector (A->B)
 *  @param[in] ap A vertex-to-point vector (A->P)
 *  @retutn The determinant value */
static fixed_56_8 calculateDeterminant(vec2fix_56_8 ab, vec2fix_56_8 ap);

/** Check if a triangle's edge is flat top or left. Used in rasterization rules.
 *  @param[in] edge A pointer to an edge vector (pointing from one vertex to the
 *  other, assuming counter-clockwise vertex ordering)
 *  @return Whether or not this edge is flat top or left */
static bool isEdgeFlatTopOrLeft(vec2fix_56_8 edge);

/** @} */  // ingroup Rasterization

void drawTriangle(
	const SRPFramebuffer* fb, const SRPvsOutput vertices[3],
	const SRPShaderProgram* restrict sp, size_t primitiveID
)
{
	vec2fix_56_8 positions[3], edges[3];
	// Cannot "unify" these loops since edges computation will touch uninitialized memory
	for (uint8_t i = 0; i < 3; i++)
	{
		double ndc[2];
		srpFramebufferNDCToScreenSpace(fb, vertices[i].position, ndc);
		positions[i] = (vec2fix_56_8) {
			FIXED_56_8_FROM_DOUBLE(ndc[0]),
			FIXED_56_8_FROM_DOUBLE(ndc[1])
		};
	}
	for (uint8_t i = 0; i < 3; i++)
		edges[i] = vec2fp_56_8_subtract(positions[(i+1)%3], positions[i]);

	// Points that define the bounding box
	fixed_56_8 x_min = \
		FIXED_56_8_FLOOR(MIN(positions[0].x, MIN(positions[1].x, positions[2].x))) + \
		FIXED_56_8_FROM_DOUBLE(0.5);
	fixed_56_8 y_min = \
		FIXED_56_8_FLOOR(MIN(positions[0].y, MIN(positions[1].y, positions[2].y))) + \
		FIXED_56_8_FROM_DOUBLE(0.5);
	fixed_56_8 x_max = \
		FIXED_56_8_FLOOR(MAX(positions[0].x, MAX(positions[1].x, positions[2].x))) + \
		FIXED_56_8_FROM_DOUBLE(0.5);
	fixed_56_8 y_max = \
		FIXED_56_8_FLOOR(MAX(positions[0].y, MAX(positions[1].y, positions[2].y))) + \
		FIXED_56_8_FROM_DOUBLE(0.5);

	srpFramebufferDrawPixel(
		fb, FIXED_56_8_TO_INT(FIXED_56_8_FLOOR(x_min)),
		FIXED_56_8_TO_INT(FIXED_56_8_FLOOR(y_min)), 0, 0xFF0000FF
	);
	srpFramebufferDrawPixel(
		fb, FIXED_56_8_TO_INT(FIXED_56_8_FLOOR(x_max)),
		FIXED_56_8_TO_INT(FIXED_56_8_FLOOR(y_max)), 0, 0xFF0000FF
	);

	// Bias values for each edge: needed for top-left rasterization rule
	fixed_56_8 bias[3] = {
		isEdgeFlatTopOrLeft(edges[0]) ? FIXED_56_8_MIN_VALUE : 0,
		isEdgeFlatTopOrLeft(edges[1]) ? FIXED_56_8_MIN_VALUE : 0,
		isEdgeFlatTopOrLeft(edges[2]) ? FIXED_56_8_MIN_VALUE : 0
	};

	// Determinant values for each edge
	fixed_56_8 w[3];
	for (fixed_56_8 y = y_min; y <= y_max; y += FIXED_56_8_ONE)
	{
		for (fixed_56_8 x = x_min; x <= x_max; x += FIXED_56_8_ONE)
		{
			vec2fix_56_8 p = {x, y};

			w[0] = calculateDeterminant(edges[0], vec2fp_56_8_subtract(p, positions[0])) - bias[0];
			w[1] = calculateDeterminant(edges[1], vec2fp_56_8_subtract(p, positions[1])) - bias[1];
			w[2] = calculateDeterminant(edges[2], vec2fp_56_8_subtract(p, positions[2])) - bias[2];

			if ((w[0] | w[1] | w[2]) >= 0)
				srpFramebufferDrawPixel(
					fb, FIXED_56_8_TO_INT(FIXED_56_8_FLOOR(x)),
					FIXED_56_8_TO_INT(FIXED_56_8_FLOOR(y)), 0, 0xFFFFFFFF
				);
		}
	}
}

static fixed_56_8 calculateDeterminant(vec2fix_56_8 ab, vec2fix_56_8 ap)
{
	return ab.y * ap.x - ab.x * ap.y;
}

static bool isEdgeFlatTopOrLeft(vec2fix_56_8 edge)
{
	bool flatTop = edge.x < 0 && edge.y == 0;
	bool left = edge.y > 0;
	return flatTop || left;
}

