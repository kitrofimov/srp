// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Primitive assembly implementation */

#include <assert.h>
#include "primitive_assembly.h"
#include "vertex_processing.h"
#include "topology.h"
#include "message_callback_p.h"
#include "context.h"
#include "arena_p.h"
#include "utils.h"

/** Assemble a single triangle
 *  @param[in] rawTriIdx Index of the primitive to assemble, starting from 0,
 * 						 including culled triangles
 *  @param[in] baseVertex Base vertex for the post-VS cache, i.e. minimal vertex
 * 						  index found in this draw call
 *  @param[in] cache Pointer to the vertex cache buffer
 *  @param[in] varyingBuffer Pointer to the buffer where vertex shader outputs will be stored
 *  @param[out] tri Pointer to the triangle where the assembled triangle will be stored
 *  @returns `true` if successful, `false` otherwise
 *  @see assembleTriangles() for documentation on other parameters */
static bool assembleOneTriangle(
    const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
    const SRPFramebuffer* fb, SRPPrimitive prim, size_t startIndex, size_t rawTriIdx,
    VertexCache* cache, void* varyingBuffer, SRPTriangle* tri
);

/** Check if draw call for a triangle is valid
 *  @see assembleTriangles() for full parameter documentation */
static bool validateTriangleDrawCall(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb,
	SRPPrimitive prim, size_t startIndex, size_t vertexCount
);

/** Allocate buffers needed for assembling triangles
 *  @param[in] nTriangles Number of triangles to allocate buffers for
 *  @param[in] sp Shader program to use
 *  @param[in] minVI Minimal vertex index found in this draw call
 *  @param[in] maxVI Maximal vertex index found in this draw call. Both with
 * 					 minVI used to determine the size of post-VS cache
 *  @param[out] outTriangles Will contain pointer to the array of assembled triangles
 *  @param[out] outVaryingBuffer Will contain pointer to the interpolated variables buffer */
static void allocateTriangleBuffers(
	size_t nTriangles, const SRPShaderProgram* sp, VertexCache* cache,
	SRPTriangle** outTriangles, void** outVaryingBuffer
);

static void allocateLineBuffers(
	size_t nLines, const SRPShaderProgram* sp, VertexCache* cache,
	SRPLine** outLines, void** outVaryingBuffer
);

static void assembleOneLine(
    const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
    const SRPFramebuffer* fb, SRPPrimitive prim, size_t startIndex, size_t rawLineIdx,
    size_t vertexCount, VertexCache* cache, void* varyingBuffer, SRPLine* line
);

bool assembleTriangles(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive prim, size_t startIndex, size_t vertexCount,
	size_t* outTriangleCount, SRPTriangle** outTriangles
)
{
	if (!validateTriangleDrawCall(ib, vb, prim, startIndex, vertexCount))
		return false;

	size_t nTriangles = computeTriangleCount(vertexCount, prim);
	if (nTriangles == 0)
		return false;

	VertexCache cache;
	SRPTriangle* triangles;
	void* varyingBuffer;
	allocateVertexCache(&cache, ib, startIndex, vertexCount);
	allocateTriangleBuffers(nTriangles, sp, &cache, &triangles, &varyingBuffer);

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

static bool validateTriangleDrawCall(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb,
	SRPPrimitive prim, size_t startIndex, size_t vertexCount
)
{
	if (prim == SRP_PRIM_TRIANGLES && vertexCount % 3 != 0)
	{
		srpMessageCallbackHelper(
			SRP_MESSAGE_WARNING, SRP_MESSAGE_SEVERITY_LOW, __func__,
			"Vertex count not divisible by 3. The last %i vertex/vertices will be ignored\n", vertexCount % 3
		);
	}

	// The last stream index that will be touched
	const size_t endIndex = startIndex + vertexCount - 1;
	const size_t bufferSize = (ib) ? ib->nIndices : vb->nVertices;

	if (endIndex >= bufferSize)
	{
		const char* errorMessage = (ib) ? \
			"Attempt to OOB access index buffer (read) at indices %zu-%zu (size: %zu)\n" : \
			"Attempt to OOB access vertex buffer (read) at indices %zu-%zu (size: %zu)\n";
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			errorMessage, startIndex, endIndex, bufferSize
		);
		return false;
	}

	return true;
}

static void allocateTriangleBuffers(
	size_t nTriangles, const SRPShaderProgram* sp, VertexCache* cache,
	SRPTriangle** outTriangles, void** outVaryingBuffer
)
{
	size_t nUniqueVertices = cache->size;
	size_t trianglesBufferSize = sizeof(SRPTriangle) * nTriangles;
	size_t varyingBufferSize = sp->vs->nBytesPerOutputVariables * nUniqueVertices;

	*outTriangles = ARENA_ALLOC(trianglesBufferSize);
	*outVaryingBuffer = ARENA_ALLOC(varyingBufferSize);
}

bool assembleLines(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive prim, size_t startIndex, size_t vertexCount,
	size_t* outLineCount, SRPLine** outLines
)
{
	size_t nLines = computeLineCount(vertexCount, prim);
	if (nLines == 0)
		return false;

	VertexCache cache;
	SRPLine* lines;
	void* varyingBuffer;
	allocateVertexCache(&cache, ib, startIndex, vertexCount);
	allocateLineBuffers(nLines, sp, &cache, &lines, &varyingBuffer);

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

static void allocateLineBuffers(
	size_t nLines, const SRPShaderProgram* sp, VertexCache* cache,
	SRPLine** outLines, void** outVaryingBuffer
)
{
	size_t nUniqueVertices = cache->size;
	size_t linesBufferSize = sizeof(SRPLine) * nLines;
	size_t varyingBufferSize = sp->vs->nBytesPerOutputVariables * nUniqueVertices;

	*outLines = ARENA_ALLOC(linesBufferSize);
	*outVaryingBuffer = ARENA_ALLOC(varyingBufferSize);
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
