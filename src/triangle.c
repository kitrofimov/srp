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

static int calculateDeterminant(const int a[2], const int b[2], const int p[2]);

/** Check if a triangle's edge is flat top or left. Used in rasterization rules.
 *  @param[in] edgeVector A pointer to an edge vector (pointing from one vertex
 *  to the other)
 *  @return Whether or not this edge is flat top or left
 *  @todo Does it matter if I pass edge 0->1 or 1->0 (numbers = vertex indices)?
 *        This should be documented. */
static bool triangleIsEdgeFlatTopOrLeft(const vec3d* restrict edgeVector);

/** @} */  // ingroup Rasterization

void drawTriangle(
	const SRPFramebuffer* fb, const SRPvsOutput vertices[3],
	const SRPShaderProgram* restrict sp, size_t primitiveID
)
{
	vec2i positions[3];
	for (uint8_t i = 0; i < 3; i++)
		srpFramebufferNDCToScreenSpace(fb, vertices[i].position, (int*) &positions[i]);

	// Points that define the bounding box
	uint32_t x_min = MIN(positions[0].x, MIN(positions[1].x, positions[2].x));
	uint32_t y_min = MIN(positions[0].y, MIN(positions[1].y, positions[2].y));
	uint32_t x_max = MAX(positions[0].x, MAX(positions[1].x, positions[2].x));
	uint32_t y_max = MAX(positions[0].y, MAX(positions[1].y, positions[2].y));

	for (uint32_t y = y_min; y <= y_max; y++)
	{
		for (uint32_t x = x_min; x <= x_max; x++)
		{
			int p[2] = {x, y};
			double w[3] = {
				calculateDeterminant((int*) &positions[0], (int*) &positions[1], p),
				calculateDeterminant((int*) &positions[1], (int*) &positions[2], p),
				calculateDeterminant((int*) &positions[2], (int*) &positions[0], p),
			};
			if (w[0] >= 0 && w[1] >= 0 && w[2] >= 0)
				srpFramebufferDrawPixel(fb, x, y, 0, 0xFFFFFFFF);
		}
	}
}

static int calculateDeterminant(const int a[2], const int b[2], const int p[2])
{
	int ab[] = {b[0] - a[0], b[1] - a[1]};
	int ap[] = {p[0] - a[0], p[1] - a[1]};
	return ab[1] * ap[0] - ab[0] * ap[1];
}

