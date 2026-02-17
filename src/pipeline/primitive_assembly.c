// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Primitive assembly implementation */

#include <assert.h>
#include "pipeline/primitive_assembly.h"
#include "pipeline/vertex_processing.h"
#include "pipeline/topology.h"
#include "pipeline/clipping.h"
#include "utils/message_callback_p.h"
#include "srp/context.h"
#include "memory/arena_p.h"
#include "utils/voidptr.h"

/** Assemble a single triangle
 *  @param[in] rawLineIdx Index of the primitive to assemble, starting from 0,
 * 						  including skipped lines
 *  @param[in] cache Pointer to the post-VS cache
 *  @param[in] varyingBuffer Buffer where vertex shader outputs will be stored
 *  @param[out] line Assembled line
 *  @returns `true` if successful, `false` otherwise
 *  @see assembleLines() for documentation on other parameters */
static void assembleOneLine(
    const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
    const SRPFramebuffer* fb, SRPPrimitive prim, size_t startIndex, size_t rawLineIdx,
    size_t vertexCount, VertexCache* cache, SRPLine* line
);

/** Check if there are excess vertices when drawing some kind of primitive,
 * 	send a warning if so
 *  @see assembleTriangles() for full parameter documentation */
static void warnOnExcessVertexCount(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb,
	SRPPrimitive prim, size_t startIndex, size_t vertexCount
);

bool assembleTriangles(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive prim, size_t startIndex, size_t vertexCount,
	size_t* outTriangleCount, SRPTriangle** outTriangles
)
{
	warnOnExcessVertexCount(ib, vb, prim, startIndex, vertexCount);
	size_t nUnclipped = computeTriangleCount(vertexCount, prim);
	if (nUnclipped == 0)
		return false;

	VertexCache cache;
	allocateVertexCache(&cache, ib, startIndex, vertexCount);

	SRPTriangle clipped[4];  // Worst-case after clipping
	SRPTriangle* triangles = ARENA_ALLOC(nUnclipped * 4 * sizeof(SRPTriangle));
	SRPTriangle* cur = triangles;

	size_t primitiveID = 0;
	for (size_t k = 0; k < nUnclipped; k += 1)
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
			SRPTriangle* dst = cur;
			*dst = clipped[i];
			bool success = setupTriangle(dst, fb);
			if (!success)
				continue;
			dst->id = primitiveID;
			primitiveID++;
			cur++;
		}
	}

	*outTriangleCount = primitiveID;  // Total amount of triangles
	*outTriangles = triangles;

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
		assembleOneLine(ib, vb, sp, fb, prim, startIndex, k, vertexCount, &cache, line);

		line->id = primitiveID;
		primitiveID++;
	}

	*outLineCount = primitiveID;
	*outLines = lines;
	return true;
}

static void assembleOneLine(
    const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
    const SRPFramebuffer* fb, SRPPrimitive prim, size_t startIndex, size_t rawLineIdx,
    size_t vertexCount, VertexCache* cache, SRPLine* line
)
{
	size_t streamIndices[2];
	resolveLineTopology(startIndex, rawLineIdx, prim, vertexCount, streamIndices);

	for (uint8_t i = 0; i < 2; i++)
	{
		size_t vertexIndex = (ib) ? indexIndexBuffer(ib, streamIndices[i]) : streamIndices[i];
		line->v[i] = *vertexCacheFetch(cache, vertexIndex, vb, sp);
	}

	setupLine(line, fb);
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

		p->id = primitiveID;
		primitiveID++;
	}

	*outPointCount = primitiveID;
	*outPoints = points;
	return true;
}
