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
static int calculateDeterminant(vec2i ab, vec2i ap);

/** Check if a triangle's edge is flat top or left. Used in rasterization rules.
 *  @param[in] edge A pointer to an edge vector (pointing from one vertex to the
 *  other, assuming counter-clockwise vertex ordering)
 *  @return Whether or not this edge is flat top or left */
static bool isEdgeFlatTopOrLeft(vec2i edge);

/** @} */  // ingroup Rasterization

void drawTriangle(
	const SRPFramebuffer* fb, const SRPvsOutput vertices[3],
	const SRPShaderProgram* restrict sp, size_t primitiveID
)
{
	vec2i positions[3], edges[3];
	// Cannot "unify" these loops since edges computation will touch uninitialized memory
	for (uint8_t i = 0; i < 3; i++)
		srpFramebufferNDCToScreenSpace(fb, vertices[i].position, (int*) &positions[i]);
	for (uint8_t i = 0; i < 3; i++)
		edges[i] = vec2iSubtract(positions[(i+1)%3], positions[i]);

	// Points that define the bounding box
	uint32_t x_min = MIN(positions[0].x, MIN(positions[1].x, positions[2].x));
	uint32_t y_min = MIN(positions[0].y, MIN(positions[1].y, positions[2].y));
	uint32_t x_max = MAX(positions[0].x, MAX(positions[1].x, positions[2].x));
	uint32_t y_max = MAX(positions[0].y, MAX(positions[1].y, positions[2].y));

	// Bias values to each edge: needed for top-left rasterization rule
	int bias[3] = {
		isEdgeFlatTopOrLeft(edges[0]) ? -1 : 0,
		isEdgeFlatTopOrLeft(edges[1]) ? -1 : 0,
		isEdgeFlatTopOrLeft(edges[2]) ? -1 : 0
	};

	// Determinant values for each edge
	double w[3];
	for (uint32_t y = y_min; y <= y_max; y++)
	{
		for (uint32_t x = x_min; x <= x_max; x++)
		{
			vec2i p = {x, y};

			w[0] = calculateDeterminant(edges[0], vec2iSubtract(p, positions[0])) + bias[0];
			w[1] = calculateDeterminant(edges[1], vec2iSubtract(p, positions[1])) + bias[1];
			w[2] = calculateDeterminant(edges[2], vec2iSubtract(p, positions[2])) + bias[2];

			if (w[0] >= 0 && w[1] >= 0 && w[2] >= 0)
				srpFramebufferDrawPixel(fb, x, y, 0, 0xFFFFFFFF);
		}
	}
}

static int calculateDeterminant(vec2i ab, vec2i ap)
{
	return ab.y * ap.x - ab.x * ap.y;
}

static bool isEdgeFlatTopOrLeft(vec2i edge)
{
	bool flatTop = edge.x < 0 && edge.y == 0;
	bool left = edge.y > 0;
	return flatTop || left;
}

