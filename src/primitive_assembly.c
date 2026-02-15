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
} VertexCacheEntry;

/** Assemble a single triangle.
 *  @param[in] triangleIdx Index of the primitive to assemble, starting from 0,
 * 						   including culled triangles
 *  @param[in] baseVertex Base vertex for the post-VS cache, i.e. minimal vertex
 * 						  index found in this draw call
 *  @param[in] cache Pointer to the vertex cache buffer
 *  @param[in] varyingBuffer Pointer to the buffer where vertex shader outputs will be stored
 *  @param[out] tri Pointer to the triangle where the assembled triangle will be stored
 *  @returns `true` if successful, `false` otherwise
 *  @see assembleTriangles() for documentation on other parameters */
static bool assembleOneTriangle(
    const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
    const SRPFramebuffer* fb, SRPPrimitive prim, size_t startIndex, size_t triangleIdx,
    size_t baseVertex, VertexCacheEntry* cache, void* varyingBuffer, SRPTriangle* tri
);

/** Fetch vertex shader output from post-VS cache. If not found, compute and store it.
 * 	@see assembleOneTriangle() for documentation on other parameters */
static SRPvsOutput* vertexCacheFetch(
	VertexCacheEntry* cache, size_t vertexIndex, size_t baseVertex,
	void* varyingBuffer, const SRPVertexBuffer* vb, const SRPShaderProgram* sp
);

/** Compute minimal and maximal vertex indices given stream indices.
 *  @see assembleOneTriangle() for documentation on other parameters */
static void computeMinMaxVI(
	const SRPIndexBuffer* ib, size_t startIndex, size_t vertexCount,
	size_t* outMinVI, size_t* outMaxVI
);

/** Check if draw call for a triangle is valid.
 *  @see assembleTriangles() for full parameter documentation */
static bool validateTriangleDrawCall(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb,
	SRPPrimitive prim, size_t startIndex, size_t vertexCount
);

/** Allocate buffers needed for assembling triangles.
 *  @param[in] nTriangles Number of triangles to allocate buffers for
 *  @param[in] sp Shader program to use
 *  @param[in] minVI Minimal vertex index found in this draw call
 *  @param[in] maxVI Maximal vertex index found in this draw call. Both with
 * 					 minVI used to determine the size of post-VS cache
 *  @param[out] outTriangles Will contain pointer to the array of assembled triangles
 *  @param[out] outVaryingBuffer Will contain pointer to the interpolated variables buffer */
static void allocateTriangleBuffers(
	size_t nTriangles, const SRPShaderProgram* sp, size_t minVI, size_t maxVI,
	SRPTriangle** outTriangles, void** outVaryingBuffer, VertexCacheEntry** outVertexCache
);

/** Deduce the number of triangles to assemble based on the number of vertices
 *  and primitive type.
 *  @param[in] vertexCount Number of vertices
 *  @param[in] prim Primitive type
 *  @return Number of triangles that should be assembled */
static size_t computeTriangleCount(size_t vertexCount, SRPPrimitive prim);
 
/** Determine the stream indices based on the triangle type.
 *  @param[in] base Base stream index
 *  @param[in] primID Primitive ID
 *  @param[in] prim Primitive type
 *  @param[out] out Array of size 3 to store the resulting stream indices */
static void resolveTriangleTopology(size_t base, size_t primID, SRPPrimitive prim, size_t* out);

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

	size_t minVI, maxVI;
	computeMinMaxVI(ib, startIndex, vertexCount, &minVI, &maxVI);

	SRPTriangle* triangles;
	void* pVarying;
	VertexCacheEntry* cache;
	allocateTriangleBuffers(nTriangles, sp, minVI, maxVI, &triangles, &pVarying, &cache);

	size_t primitiveID = 0;
	for (size_t k = 0; k < nTriangles; k += 1)
	{
		SRPTriangle* tri = &triangles[primitiveID];
		bool success = assembleOneTriangle(
			ib, vb, sp, fb, prim, startIndex, k, minVI, cache, pVarying, tri
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
    const SRPFramebuffer* fb, SRPPrimitive prim, size_t startIndex, size_t triangleIdx,
    size_t baseVertex, VertexCacheEntry* cache, void* varyingBuffer, SRPTriangle* tri
)
{
    size_t streamIndices[3];
    resolveTriangleTopology(startIndex, triangleIdx, prim, streamIndices);

    for (uint8_t i = 0; i < 3; i++)
    {
        size_t vertexIndex = (ib) ? indexIndexBuffer(ib, streamIndices[i]) : streamIndices[i];
		tri->v[i] = *vertexCacheFetch(cache, vertexIndex, baseVertex, varyingBuffer, vb, sp);
	}

	bool success = setupTriangle(tri, fb);
    return success;
}

static SRPvsOutput* vertexCacheFetch(
	VertexCacheEntry* cache, size_t vertexIndex, size_t baseVertex,
	void* varyingBuffer, const SRPVertexBuffer* vb, const SRPShaderProgram* sp
)
{
	VertexCacheEntry* entry = &cache[vertexIndex - baseVertex];
	const size_t stride = sp->vs->nBytesPerOutputVariables;

	if (!entry->valid)
	{
		SRPVertex* pVertex = indexVertexBuffer(vb, vertexIndex);
		void* pVarying = INDEX_VOID_PTR(varyingBuffer, vertexIndex - baseVertex, stride);

		SRPvsInput vsIn = {
			.vertexID = vertexIndex,
			.pVertex  = pVertex,
			.uniform  = sp->uniform
		};
		entry->data = (SRPvsOutput) {
			.position = {0},
			.pOutputVariables = pVarying
		};

		sp->vs->shader(&vsIn, &entry->data);

		// Perspective divide
		double invW = 1.0 / entry->data.position[3];
		entry->data.position[0] *= invW;
		entry->data.position[1] *= invW;
		entry->data.position[2] *= invW;
		entry->data.position[3] = 1.0;

		entry->valid = true;
	}

	return &entry->data;
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
	size_t nTriangles, const SRPShaderProgram* sp, size_t minVI, size_t maxVI,
	SRPTriangle** outTriangles, void** outVaryingBuffer, VertexCacheEntry** outVertexCache
)
{
	size_t nUniqueVertices = maxVI - minVI + 1;
	size_t trianglesBufferSize = sizeof(SRPTriangle) * nTriangles;
	size_t varyingBufferSize = sp->vs->nBytesPerOutputVariables * nUniqueVertices;
	size_t vertexCacheSize = sizeof(VertexCacheEntry) * nUniqueVertices;

	*outTriangles = ARENA_ALLOC(trianglesBufferSize);
	*outVaryingBuffer = ARENA_ALLOC(varyingBufferSize);
	*outVertexCache = ARENA_CALLOC(vertexCacheSize);
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

static void resolveTriangleTopology(size_t base, size_t primID, SRPPrimitive prim, size_t* out)
{
	if (prim == SRP_PRIM_TRIANGLES)
	{
		out[0] = base + primID * 3;
		out[1] = base + primID * 3 + 1;
		out[2] = base + primID * 3 + 2;
	}
	else if (prim == SRP_PRIM_TRIANGLE_STRIP)
	{
		// Maintain proper winding order for odd triangles (swap the first two vertices)
		bool odd = (primID % 2 == 1);
		out[0] = base + primID + ((odd) ? 1 : 0);
		out[1] = base + primID + ((odd) ? 0 : 1);
		out[2] = base + primID + 2;
	}
	else if (prim == SRP_PRIM_TRIANGLE_FAN)
	{
		out[0] = base;
		out[1] = base + primID + 1;
		out[2] = base + primID + 2;
	}
	else
		assert(false && "Invalid primitive type");
}

bool assemblePoints(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, size_t startIndex, size_t count, SRPPoint** outPoints
)
{
	if (srpContext.pointSize <= 0.)
		return false;

	const size_t stride = sp->vs->nBytesPerOutputVariables;
	SRPPoint* points = ARENA_ALLOC(sizeof(SRPPoint) * count);
	void* varyingBuffer = ARENA_ALLOC(stride * count);
	const size_t nPoints = count;

	for (size_t i = 0; i < nPoints; i += 1)
	{
		SRPPoint* p = &points[i];
		p->id = i;

		size_t vertexIndex = (ib) ? indexIndexBuffer(ib, startIndex+i) : startIndex+i;
		SRPVertex* pVertex = indexVertexBuffer(vb, vertexIndex);
		void* pVarying = INDEX_VOID_PTR(varyingBuffer, vertexIndex, stride);

		SRPvsInput vsIn = {
			.vertexID = vertexIndex,
			.pVertex  = pVertex,
			.uniform  = sp->uniform
		};
		p->v = (SRPvsOutput) {
			.position = {0},
			.pOutputVariables = pVarying
		};

		sp->vs->shader(&vsIn, &p->v);

		// Perspective divide
		double invW = 1.0 / p->v.position[3];
		p->v.position[0] *= invW;
		p->v.position[1] *= invW;
		p->v.position[2] *= invW;
		p->v.position[3] = 1.0;
	}

	*outPoints = points;
	return true;
}
