// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Topology-related functions implementation */

#include <assert.h>
#include "topology.h"

size_t computeTriangleCount(size_t vertexCount, SRPPrimitive prim)
{
	if (prim == SRP_PRIM_TRIANGLES)
		return vertexCount / 3;
	else if (prim == SRP_PRIM_TRIANGLE_STRIP || prim == SRP_PRIM_TRIANGLE_FAN)
		return (vertexCount >= 3) ? vertexCount - 2 : 0;
	else
		assert(false && "Invalid primitive type");
}

void resolveTriangleTopology(size_t base, size_t rawTriIdx, SRPPrimitive prim, size_t* out)
{
	if (prim == SRP_PRIM_TRIANGLES)
	{
		out[0] = base + rawTriIdx * 3;
		out[1] = base + rawTriIdx * 3 + 1;
		out[2] = base + rawTriIdx * 3 + 2;
	}
	else if (prim == SRP_PRIM_TRIANGLE_STRIP)
	{
		// Maintain proper winding order for odd triangles (swap the first two vertices)
		bool odd = (rawTriIdx % 2 == 1);
		out[0] = base + rawTriIdx + ((odd) ? 1 : 0);
		out[1] = base + rawTriIdx + ((odd) ? 0 : 1);
		out[2] = base + rawTriIdx + 2;
	}
	else if (prim == SRP_PRIM_TRIANGLE_FAN)
	{
		out[0] = base;
		out[1] = base + rawTriIdx + 1;
		out[2] = base + rawTriIdx + 2;
	}
	else
		assert(false && "Invalid primitive type");
}

size_t computeLineCount(size_t vertexCount, SRPPrimitive prim)
{
	if (prim == SRP_PRIM_LINES)
		return (vertexCount % 2 == 0) ? vertexCount / 2 : (vertexCount-1) / 2;
	else if (prim == SRP_PRIM_LINE_STRIP)
		return (vertexCount != 1) ? vertexCount-1 : 0;
	else if (prim == SRP_PRIM_LINE_LOOP)
		return (vertexCount != 1) ? vertexCount : 0;
	else
		assert(false && "Invalid primitive type");
}

void resolveLineTopology(
	size_t base, size_t rawLineIdx, SRPPrimitive prim, size_t vertexCount, size_t* out
)
{
	if (prim == SRP_PRIM_LINES)
	{
		out[0] = base + rawLineIdx * 2;
		out[1] = base + rawLineIdx * 2 + 1;
	}
	else if (prim == SRP_PRIM_LINE_STRIP)
	{
		out[0] = base + rawLineIdx;
		out[1] = base + rawLineIdx + 1;
	}
	else if (prim == SRP_PRIM_LINE_LOOP)
	{
		out[0] = base + rawLineIdx;
		out[1] = base + ((rawLineIdx + 1) % vertexCount);
	}
	else
		assert(false && "Invalid primitive type");
}
