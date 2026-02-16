// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Primitive assembly implementation */

#include <assert.h>
#include "pipeline/primitive_assembly.h"
#include "pipeline/vertex_processing.h"
#include "pipeline/topology.h"
#include "utils/message_callback_p.h"
#include "srp/context.h"
#include "memory/arena_p.h"
#include "utils/voidptr.h"

/** Assemble a single triangle
 *  @param[in] rawTriIdx Index of the primitive to assemble, starting from 0,
 * 						 including culled triangles
 *  @param[in] cache Pointer to the post-VS cache
 *  @param[in] varyingBuffer Buffer where vertex shader outputs will be stored
 *  @param[out] tri Assembled triangle
 *  @returns `true` if successful, `false` otherwise
 *  @see assembleTriangles() for documentation on other parameters */
static bool assembleOneTriangle(
    const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
    const SRPFramebuffer* fb, SRPPrimitive prim, size_t startIndex, size_t rawTriIdx,
    VertexCache* cache, void* varyingBuffer, SRPTriangle* tri
);

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
    size_t vertexCount, VertexCache* cache, void* varyingBuffer, SRPLine* line
);

/** Allocate buffers needed for assembling primitives
 *  @param[in] nPrimitives Number of primitives to allocate buffers for
 *  @param[in] primSize Size of each primitive, in bytes
 *  @param[in] sp Shader program to use
 *  @param[in] cache Post-VS cache that is going to be used
 *  @param[out] outPrimitives Will contain pointer to the array of assembled primitives
 *  @param[out] outVaryingBuffer Will contain pointer to the interpolated variables buffer */
static void allocateBuffers(
    size_t nPrimitives, size_t primSize, const SRPShaderProgram* sp, 
    VertexCache* cache, void** outPrimitives, void** outVaryingBuffer
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
	size_t nTriangles = computeTriangleCount(vertexCount, prim);
	if (nTriangles == 0)
		return false;

	VertexCache cache;
	SRPTriangle* triangles;
	void* varyingBuffer;
	allocateVertexCache(&cache, ib, startIndex, vertexCount);
	allocateBuffers(nTriangles, sizeof(SRPTriangle), sp, &cache, (void**) &triangles, &varyingBuffer);

	size_t primitiveID = 0;
	for (size_t k = 0; k < nTriangles; k += 1)
	{
		SRPTriangle* tri = &triangles[primitiveID];
		bool success = assembleOneTriangle(
			ib, vb, sp, fb, prim, startIndex, k, &cache, varyingBuffer, tri
		);

		if (!success)
			continue;

		tri->id = primitiveID;
		primitiveID++;
	}

	*outTriangleCount = primitiveID;  // Amount of non-culled triangles
	*outTriangles = triangles;
	return true;
}

static bool assembleOneTriangle(
    const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
    const SRPFramebuffer* fb, SRPPrimitive prim, size_t startIndex, size_t rawTriIdx,
    VertexCache* cache, void* varyingBuffer, SRPTriangle* tri
)
{
    size_t streamIndices[3];
    resolveTriangleTopology(startIndex, rawTriIdx, prim, streamIndices);

    for (uint8_t i = 0; i < 3; i++)
    {
        size_t vertexIndex = (ib) ? indexIndexBuffer(ib, streamIndices[i]) : streamIndices[i];
		VertexCacheEntry* entry = vertexCacheFetch(cache, vertexIndex, varyingBuffer, vb, sp);
		tri->v[i] = entry->data;
		tri->invW[i] = entry->invW;
	}

	bool success = setupTriangle(tri, fb);
    return success;
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
	SRPLine* lines;
	void* varyingBuffer;
	allocateVertexCache(&cache, ib, startIndex, vertexCount);
	allocateBuffers(nLines, sizeof(SRPLine), sp, &cache, (void**) &lines, &varyingBuffer);

	size_t primitiveID = 0;
	for (size_t k = 0; k < nLines; k += 1)
	{
		SRPLine* line = &lines[primitiveID];
		assembleOneLine(
			ib, vb, sp, fb, prim, startIndex, k, vertexCount,
			&cache, varyingBuffer, line
		);

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
    size_t vertexCount, VertexCache* cache, void* varyingBuffer, SRPLine* line
)
{
	size_t streamIndices[2];
	resolveLineTopology(startIndex, rawLineIdx, prim, vertexCount, streamIndices);

	for (uint8_t i = 0; i < 2; i++)
	{
		size_t vertexIndex = (ib) ? indexIndexBuffer(ib, streamIndices[i]) : streamIndices[i];
		VertexCacheEntry* entry = vertexCacheFetch(cache, vertexIndex, varyingBuffer, vb, sp);
		line->v[i] = entry->data;
		line->invW[i] = entry->invW;
	}

	setupLine(line, fb);
}

static void allocateBuffers(
    size_t nPrimitives, size_t primSize, const SRPShaderProgram* sp, 
    VertexCache* cache, void** outPrimitives, void** outVaryingBuffer
) {
	size_t nUniqueVertices = cache->size;
	size_t trianglesBufferSize = primSize * nPrimitives;
	size_t varyingBufferSize = sp->vs->nBytesPerOutputVariables * nUniqueVertices;

	*outPrimitives = ARENA_ALLOC(trianglesBufferSize);
	*outVaryingBuffer = ARENA_ALLOC(varyingBufferSize);
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

	const size_t stride = sp->vs->nBytesPerOutputVariables;
	const size_t nPoints = count;

	SRPPoint* points = ARENA_ALLOC(sizeof(SRPPoint) * nPoints);
	void* varyingBuffer = ARENA_ALLOC(stride * nPoints);

	size_t primitiveID = 0;
	for (size_t k = 0; k < nPoints; k += 1)
	{
		SRPPoint* p = &points[primitiveID];

		size_t vertexIndex = (ib) ? indexIndexBuffer(ib, startIndex+k) : startIndex+k;
		processVertex(
			vertexIndex, k, varyingBuffer, vb, sp,
			&p->v, NULL
		);

		p->id = primitiveID;
		primitiveID++;
	}

	*outPointCount = primitiveID;
	*outPoints = points;
	return true;
}
