// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Primitive assembly implementation */

#include <assert.h>
#include "primitive_assembly.h"
#include "message_callback_p.h"
#include "context.h"
#include "arena_p.h"
#include "utils.h"

/** Represents an entry in the post-VS cache */
typedef struct VertexCacheEntry {
	bool valid;
	SRPvsOutput data;
	double invW;
} VertexCacheEntry;

typedef struct VertexCache {
	VertexCacheEntry* entries;
	size_t baseVertex;
	size_t size;
} VertexCache;

/** Allocate post-VS vertex cache */
static void allocateVertexCache(
	VertexCache* cache, const SRPIndexBuffer* ib, size_t startIndex, size_t vertexCount
);

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

/** Fetch vertex shader output from post-VS cache. If not found, compute and store it
 * 	@see assembleOneTriangle() for documentation on other parameters */
static VertexCacheEntry* vertexCacheFetch(
	VertexCache* cache, size_t vertexIndex, void* varyingBuffer,
	const SRPVertexBuffer* vb, const SRPShaderProgram* sp
);

/** Run vertex shader and perform perspective divide */
static void processVertex(
	size_t vertexIndex, size_t varyingIndex, void* varyingBuffer,
	const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
	SRPvsOutput* outV, double* outInvW
);

/** Apply perspective divide to the output of the vertex shader,
 *  saving the 1 / W_clip value
 *  @param[in] output Output of the vertex shader
 *  @param[out] outInvW Pointer where 1/W value will be stored. May be NULL */
static void applyPerspectiveDivide(SRPvsOutput* output, double* outInvW);

/** Compute minimal and maximal vertex indices given stream indices
 *  @see assembleOneTriangle() for documentation on other parameters */
static void computeMinMaxVI(
	const SRPIndexBuffer* ib, size_t startIndex, size_t vertexCount,
	size_t* outMinVI, size_t* outMaxVI
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

/** Deduce the number of triangles to assemble based on the number of vertices
 *  and primitive type
 *  @param[in] vertexCount Number of vertices
 *  @param[in] prim Primitive type
 *  @return Number of triangles that should be assembled */
static size_t computeTriangleCount(size_t vertexCount, SRPPrimitive prim);
 
/** Determine the stream indices for a triangle based on its type
 *  @param[in] base Base stream index
 *  @param[in] rawTriIdx Index of the primitive to assemble, starting from 0,
 * 						 including culled triangles
 *  @param[in] prim Primitive type
 *  @param[out] out Array of size 3 to store the resulting stream indices */
static void resolveTriangleTopology(
	size_t base, size_t rawTriIdx, SRPPrimitive prim, size_t* out
);

/** Deduce the number of lines to assemble based on the number of vertices
 *  and primitive type */
static size_t computeLineCount(size_t vertexCount, SRPPrimitive prim);

/** Determine the stream indices for a line based on its type
 *  @param[in] base Base stream index
 *  @param[in] rawLineIdx Index of the primitive to assemble, starting from 0,
 * 						  including culled/skipped lines
 *  @param[in] prim Primitive type
 *  @param[in] vertexCount The total amount of stream vertices for this draw call
 *  @param[out] out Array of size 2 to store the resulting stream indices */
static void resolveLineTopology(
	size_t base, size_t rawLineIdx, SRPPrimitive prim, size_t vertexCount, size_t* out
);

/** Determine if a point should be clipped or not
 *  @param[in] p Pointer to a point
 *  @return Whether this point should be clipped or not */
static bool shouldClipPoint(SRPPoint* p);

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

static void allocateVertexCache(
	VertexCache* cache, const SRPIndexBuffer* ib, size_t startIndex, size_t vertexCount
)
{
	size_t minVI, maxVI;
	computeMinMaxVI(ib, startIndex, vertexCount, &minVI, &maxVI);

	// Setting all cache to zero before using it (calloc)
	cache->baseVertex = minVI;
	cache->size = maxVI - minVI + 1;
	cache->entries = ARENA_CALLOC(sizeof(VertexCacheEntry) * cache->size);
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

static VertexCacheEntry* vertexCacheFetch(
	VertexCache* cache, size_t vertexIndex,	void* varyingBuffer,
	const SRPVertexBuffer* vb, const SRPShaderProgram* sp
)
{
	VertexCacheEntry* entry = &cache->entries[vertexIndex - cache->baseVertex];

	if (!entry->valid)
	{
		processVertex(
			vertexIndex, vertexIndex - cache->baseVertex, varyingBuffer, vb, sp,
			&entry->data, &entry->invW
		);
		entry->valid = true;
	}

	return entry;
}

static void processVertex(
	size_t vertexIndex, size_t varyingIndex, void* varyingBuffer,
	const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
	SRPvsOutput* outV, double* outInvW
)
{
	SRPVertex* pVertex = indexVertexBuffer(vb, vertexIndex);
	const size_t stride = sp->vs->nBytesPerOutputVariables;
	void* pVarying = INDEX_VOID_PTR(varyingBuffer, varyingIndex, stride);

	SRPvsInput vsIn = {
		.vertexID = vertexIndex,
		.pVertex  = pVertex,
		.uniform  = sp->uniform
	};
	*outV = (SRPvsOutput) {
		.position = {0},
		.pOutputVariables = pVarying
	};

	sp->vs->shader(&vsIn, outV);
	applyPerspectiveDivide(outV, outInvW);
}

static void applyPerspectiveDivide(SRPvsOutput* output, double* outInvW)
{
    double clipW = output->position[3];
    double invW = 1.0 / clipW;
	if (outInvW != NULL)
		*outInvW = invW;

    output->position[0] *= invW;
    output->position[1] *= invW;
    output->position[2] *= invW;
    output->position[3] = 1.0;
}

static void computeMinMaxVI(
	const SRPIndexBuffer* ib, size_t startIndex, size_t vertexCount,
	size_t* outMinVI, size_t* outMaxVI
)
{
	size_t minVI = SIZE_MAX;
	size_t maxVI = 0;
	if (ib)
		for (size_t i = 0; i < vertexCount; i++)
		{
			size_t vi = indexIndexBuffer(ib, startIndex + i);
			if (vi < minVI) minVI = vi;
			if (vi > maxVI) maxVI = vi;
		}
	else
	{
		minVI = startIndex;
		maxVI = startIndex + vertexCount - 1;
	}

	*outMinVI = minVI;
	*outMaxVI = maxVI;
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

static size_t computeTriangleCount(size_t vertexCount, SRPPrimitive prim)
{
	if (prim == SRP_PRIM_TRIANGLES)
		return vertexCount / 3;
	else if (prim == SRP_PRIM_TRIANGLE_STRIP || prim == SRP_PRIM_TRIANGLE_FAN)
		return (vertexCount >= 3) ? vertexCount - 2 : 0;
	else
		assert(false && "Invalid primitive type");
}

static void resolveTriangleTopology(size_t base, size_t rawTriIdx, SRPPrimitive prim, size_t* out)
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

bool assembleLines(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive prim, size_t startIndex, size_t vertexCount,
	size_t* outLineCount, SRPLine** outLines
)
{
	const size_t stride = sp->vs->nBytesPerOutputVariables;

	size_t nLines = computeLineCount(vertexCount, prim);
	if (nLines == 0)
		return false;

	SRPLine* lines = ARENA_ALLOC(sizeof(SRPLine) * nLines);
	void* varyingBuffer = ARENA_ALLOC(stride * 2 * nLines);

	size_t primitiveID = 0;
	for (size_t k = 0; k < nLines; k += 1)
	{
		SRPLine* line = &lines[primitiveID];

		size_t streamIndices[2];
		resolveLineTopology(startIndex, k, prim, vertexCount, streamIndices);

		for (uint8_t i = 0; i < 2; i++)
		{
			size_t vertexIndex = (ib) ? indexIndexBuffer(ib, streamIndices[i]) : streamIndices[i];
			processVertex(
				vertexIndex, k+i, varyingBuffer, vb, sp,
				&line->v[i], &line->invW[i]
			);
		}

		setupLine(line, fb);

		line->id = primitiveID;
		primitiveID++;
	}

	*outLineCount = primitiveID;
	*outLines = lines;
	return true;
}

static size_t computeLineCount(size_t vertexCount, SRPPrimitive prim)
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

static void resolveLineTopology(
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

		if (shouldClipPoint(p))
			continue;

		p->id = primitiveID;
		primitiveID++;
	}

	*outPointCount = primitiveID;  // Amount of non-clipped points
	*outPoints = points;
	return true;
}

static bool shouldClipPoint(SRPPoint* p)
{
	for (int i = 0; i < 3; i++)
		if (p->v.position[i] < -1.0 || p->v.position[i] > 1.0)
			return true;
	return false;
}
