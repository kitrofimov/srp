// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Primitive assembly implementation */

#include <stdlib.h>
#include "pipeline/primitive_assembly.h"
#include "pipeline/vertex_processing.h"
#include "pipeline/topology.h"
#include "pipeline/clipping.h"
#include "utils/message_callback_p.h"
#include "srp/context.h"
#include "memory/arena_p.h"
#include "utils/voidptr.h"

/** Check if there are excess vertices when drawing some kind of primitive,
 * 	send a warning if so
 *  @see assembleTriangles() for full parameter documentation */
static void warnOnExcessVertexCount(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb,
	SRPPrimitive prim, size_t startIndex, size_t vertexCount
);

/** Calculate constants for triangle assembly */
static void resolvePolygonModeOutput(
	size_t* nOutPrimitivesPerClippedTriangle, size_t* sizeOutPrimitive
);

bool assembleTrianglesGeneric(
    const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
    const SRPShaderProgram* sp, SRPPrimitive prim, size_t startIndex, size_t vertexCount,
    size_t* outCount, void** outPrimitives
)
{
    warnOnExcessVertexCount(ib, vb, prim, startIndex, vertexCount);
    size_t nUnclipped = computeTriangleCount(vertexCount, prim);
    if (nUnclipped == 0)
    {
        *outCount = 0;
        *outPrimitives = NULL;
        return false;
    }

    VertexCache cache;
    allocateVertexCache(&cache, ib, startIndex, vertexCount);

	size_t nOutPrimitivesPerClippedTriangle, sizeOutPrimitive;
	resolvePolygonModeOutput(&nOutPrimitivesPerClippedTriangle, &sizeOutPrimitive);

    // Worst case: each triangle becomes clipped triangles
    SRPTriangle clipped[4];
    size_t maxTotal = nUnclipped * 4 * nOutPrimitivesPerClippedTriangle;
    void* buffer = ARENA_ALLOC(maxTotal * sizeOutPrimitive);
    void* cur = buffer;

    size_t primitiveID = 0;
    for (size_t k = 0; k < nUnclipped; k++)
    {
        SRPTriangle unclipped;

        size_t streamIndices[3];
        resolveTriangleTopology(startIndex, k, prim, streamIndices);

        for (uint8_t i = 0; i < 3; i++)
        {
            size_t vertexIndex = (ib) ? indexIndexBuffer(ib, streamIndices[i]) : streamIndices[i];
            unclipped.v[i] = *vertexCacheFetch(&cache, vertexIndex, vb, sp);
        }

        size_t nClipped = clipTriangle(&unclipped, sp, clipped);

        for (size_t i = 0; i < nClipped; i++)
        {
			if (srpContext.polygonMode == SRP_POLYGON_MODE_FILL)
			{
				SRPTriangle* dst = (SRPTriangle*) cur;
				*dst = *clipped;

				if (!setupTriangle(dst, fb))
					continue;

				dst->id = primitiveID;
				primitiveID++;
				cur = dst + 1;
			}
			else if (srpContext.polygonMode == SRP_POLYGON_MODE_LINE)
			{
				SRPLine* dst = (SRPLine*) cur;
				for (uint8_t j = 0; j < 3; j++)
				{
					dst->v[0] = clipped[i].v[j];
					dst->v[1] = clipped[i].v[(j + 1) % 3];

					setupLine(dst, fb);
					dst->id = primitiveID;
					primitiveID++;
					dst++;
				}
				cur = dst + 1;
			}
			else if (srpContext.polygonMode == SRP_POLYGON_MODE_POINT)
			{
				SRPPoint* dst = (SRPPoint*) cur;
				for (uint8_t j = 0; j < 3; j++)
				{
					dst->v = clipped[i].v[j];
					setupPoint(dst);
					dst->id = primitiveID;
					primitiveID++;
					dst++;
				}
				cur = dst + 1;
			}
        }
    }

    *outCount = primitiveID;
    *outPrimitives = buffer;

    return true;
}

bool assembleLines(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive prim, size_t startIndex, size_t vertexCount,
	size_t* outLineCount, SRPLine** outLines
)
{
	warnOnExcessVertexCount(ib, vb, prim, startIndex, vertexCount);
	size_t nLines = computeLineCount(vertexCount, prim);
	if (nLines == 0)
		return false;

	VertexCache cache;
	SRPLine* lines = ARENA_ALLOC(sizeof(SRPLine) * nLines);
	allocateVertexCache(&cache, ib, startIndex, vertexCount);

	size_t primitiveID = 0;
	for (size_t k = 0; k < nLines; k += 1)
	{
		SRPLine* line = &lines[primitiveID];

		size_t streamIndices[2];
		resolveLineTopology(startIndex, k, prim, vertexCount, streamIndices);

		for (uint8_t i = 0; i < 2; i++)
		{
			size_t vertexIndex = (ib) ? indexIndexBuffer(ib, streamIndices[i]) : streamIndices[i];
			line->v[i] = *vertexCacheFetch(&cache, vertexIndex, vb, sp);
		}

		if (clipLine(line, sp))  // Fully clipped
			continue;

		setupLine(line, fb);

		line->id = primitiveID;
		primitiveID++;
	}

	*outLineCount = primitiveID;
	*outLines = lines;
	return true;
}

static void warnOnExcessVertexCount(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb,
	SRPPrimitive prim, size_t startIndex, size_t vertexCount
)
{
	if (prim == SRP_PRIM_LINES && vertexCount % 2 != 0)
		srpMessageCallbackHelper(
			SRP_MESSAGE_WARNING, SRP_MESSAGE_SEVERITY_LOW, __func__,
			"Odd vertex count when drawing SRP_PRIM_LINES. The last vertex will be ignored\n"
		);

	if (prim == SRP_PRIM_TRIANGLES && vertexCount % 3 != 0)
		srpMessageCallbackHelper(
			SRP_MESSAGE_WARNING, SRP_MESSAGE_SEVERITY_LOW, __func__,
			"Vertex count not divisible by 3 when drawing SRP_PRIM_TRIANGLES. The last %i vertex/vertices will be ignored\n", vertexCount % 3
		);
}

static void resolvePolygonModeOutput(
	size_t* nOutPrimitivesPerClippedTriangle, size_t* sizeOutPrimitive
)
{
	if (srpContext.polygonMode == SRP_POLYGON_MODE_FILL)
	{
		*nOutPrimitivesPerClippedTriangle = 1;
		*sizeOutPrimitive = sizeof(SRPTriangle);
	}
	else if (srpContext.polygonMode == SRP_POLYGON_MODE_LINE)
	{
		*nOutPrimitivesPerClippedTriangle = 3;
		*sizeOutPrimitive = sizeof(SRPLine);
	}
	else if (srpContext.polygonMode == SRP_POLYGON_MODE_POINT)
	{
		*nOutPrimitivesPerClippedTriangle = 3;
		*sizeOutPrimitive = sizeof(SRPPoint);
	}
	else
		abort();
}

bool assemblePoints(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, size_t startIndex, size_t count,
	size_t* outPointCount, SRPPoint** outPoints
)
{
	if (srpContext.pointSize <= 0.)
		return false;

	const size_t nPoints = count;
	SRPPoint* points = ARENA_ALLOC(sizeof(SRPPoint) * nPoints);

	size_t primitiveID = 0;
	for (size_t k = 0; k < nPoints; k += 1)
	{
		SRPPoint* p = &points[primitiveID];

		size_t vertexIndex = (ib) ? indexIndexBuffer(ib, startIndex+k) : startIndex+k;
		processVertex(vertexIndex, k, vb, sp, &p->v);

		if (clipPoint(p))
			continue;

		setupPoint(p);

		p->id = primitiveID;
		primitiveID++;
	}

	*outPointCount = primitiveID;
	*outPoints = points;
	return true;
}
