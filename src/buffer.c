// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "context.h"
#include "arena_p.h"
#include "buffer_p.h"
#include "message_callback_p.h"
#include "triangle.h"
#include "type.h"
#include "utils.h"
#include "defines.h"
#include "vertex.h"

/** @file
 *  Buffer implementation */

/** @ingroup Buffer_internal
 *  @{ */

/** Call the vertex shader and assemble triangles from vertex or index buffer.
 *  If `ib == NULL`, assembles from vertex buffer, else from index buffer.
 *  Uses memory from SRPArena, so the returned triangles are valid until the
 *  next call to arenaReset().
 *  @param[in] ib Pointer to index buffer, or `NULL` if assembling from vertex buffer
 *  @param[in] vb Pointer to vertex buffer
 *  @param[in] fb Pointer to the framebuffer to draw to (needed for NDC to screen-space conversion)
 *  @param[in] sp Pointer to the shader program to use
 *  @param[in] primitive Primitive type (e.g. SRP_PRIM_TRIANGLES)
 *  @param[in] startIndex Index of the first vertex/index to assemble
 *  @param[in] count Number of vertices/indices to assemble
 *  @param[out] outTriangleCount Number of triangles assembled
 *  @param[out] outTriangles Pointer to the array of assembled triangles
 *  @returns `true` if successful, `false` otherwise. If `false` is returned,
 * 			 `*outTriangleCount` and `*outTriangles` are undefined */
static bool assembleTriangles(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count,
	size_t* outTriangleCount, SRPTriangle** outTriangles
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
 *  @param[out] outTriangles Will contain pointer to the array of assembled triangles
 *  @param[out] outVaryingBuffer Will contain pointer to the interpolated variables buffer */
static void allocateTriangleBuffers(
	size_t nTriangles, const SRPShaderProgram* sp,
	SRPTriangle** outTriangles, void** outVaryingBuffer
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

/** Draw either SRPIndexBuffer or SRPVertexBuffer.
 *  If `ib == NULL`, draws the vertex buffer, else draws index buffer.
 *  Created because vertex and index buffer drawing are very similar,
 *  with an intent to avoid code duplication
 *  @see srpDrawVertexBuffer() srpDrawIndexBuffer() for parameter documentation */
static void drawBuffer(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
);

/** Draw triangle-based primitives from either SRPIndexBuffer or SRPVertexBuffer.
 *  @see drawBuffer() for extensive documentation */
static void drawTriangles(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
);

/** Draw line-based primitives from either SRPIndexBuffer or SRPVertexBuffer.
 *  @see drawBuffer() for extensive documentation */
static void drawLines(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
);

/** Draw points from either SRPIndexBuffer or SRPVertexBuffer.
 *  @see drawBuffer() for extensive documentation */
static void drawPoints(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
);

/** Determine if a primitive is triangle-based.
 *  @param[in] primitive Primitive type
 *  @return `true` if the primitive is triangle-based, `false` otherwise */
static bool isPrimitiveTriangle(SRPPrimitive primitive);

/** Determine if a primitive is line-based.
 *  @param[in] primitive Primitive type
 *  @return `true` if the primitive is line-based, `false` otherwise */
static bool isPrimitiveLine(SRPPrimitive primitive);

/** Determine if a primitive is a point.
 *  @param[in] primitive Primitive type
 *  @return `true` if the primitive is a point, `false` otherwise */
static bool isPrimitivePoint(SRPPrimitive primitive);

/** Get an element stored in SRPIndexBuffer.
 *  Needed because SRPIndexBuffer stores opaque index types.
 *  @param[in] this Pointer to SRPIndexBuffer
 *  @param[in] ibIndex Index of the element in the SRPIndexBuffer
 *  @return Element upcasted to `uint64_t` */
static uint64_t indexIndexBuffer(const SRPIndexBuffer* this, size_t ibIndex);

/** Get a vertex stored in SRPVertexBuffer.
 *  @param[in] this Pointer to SRPVertexBuffer
 *  @param[in] index Index of the vertex to get
 *  @return Requested vertex */
static SRPVertex* indexVertexBuffer(const SRPVertexBuffer* this, size_t index);

/** @} */

static bool assembleTriangles(
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

	SRPTriangle* triangles;
	void* pVarying;
	allocateTriangleBuffers(nTriangles, sp, &triangles, &pVarying);

	size_t primitiveID = 0;
	size_t streamIndices[3];

	for (size_t k = 0; k < nTriangles; k += 1)
	{
		SRPTriangle* tri = &triangles[primitiveID];
		resolveTriangleTopology(startIndex, k, prim, streamIndices);

		for (uint8_t i = 0; i < 3; i++)
		{
			size_t vertexIndex = (ib) ? indexIndexBuffer(ib, streamIndices[i]) : streamIndices[i];
			SRPVertex* pVertex = indexVertexBuffer(vb, vertexIndex);

			SRPvsInput vsIn = {
				.vertexID = vertexIndex,
				.pVertex  = pVertex,
				.uniform  = sp->uniform
			};
			tri->v[i] = (SRPvsOutput) {
				.position = {0},
				.pOutputVariables = pVarying
			};
			pVarying = ADD_VOID_PTR(pVarying, sp->vs->nBytesPerOutputVariables);

			sp->vs->shader(&vsIn, &tri->v[i]);

			// Perspective divide
			double invW = 1.0f / tri->v[i].position[3];
			tri->v[i].position[0] *= invW;
			tri->v[i].position[1] *= invW;
			tri->v[i].position[2] *= invW;
			tri->v[i].position[3] = 1.0f;
		}

		bool success = setupTriangle(tri, fb);
		if (!success)  // triangle was culled
			continue;

		tri->id = primitiveID;
		primitiveID++;
	}

	*outTriangleCount = primitiveID;  // Amount of non-culled triangles
	*outTriangles = triangles;
	return true;
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
	size_t nTriangles, const SRPShaderProgram* sp,
	SRPTriangle** outTriangles, void** outVaryingBuffer
)
{
	size_t trianglesBufferSize = sizeof(SRPTriangle) * nTriangles;
	size_t varyingBufferSize = sp->vs->nBytesPerOutputVariables * 3 * nTriangles;

	*outTriangles = arenaAlloc(srpContext.arena, trianglesBufferSize);
	*outVaryingBuffer = arenaAlloc(srpContext.arena, varyingBufferSize);
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

static void drawBuffer(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
)
{
	if (count == 0)
		return;

	if (isPrimitiveTriangle(primitive))
		drawTriangles(ib, vb, fb, sp, primitive, startIndex, count);
	else if (isPrimitiveLine(primitive))
		drawLines(ib, vb, fb, sp, primitive, startIndex, count);
	else if (isPrimitivePoint(primitive))
		drawPoints(ib, vb, fb, sp, primitive, startIndex, count);
	else
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Unknown primitive type: %i", primitive
		);
}

static void drawTriangles(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
)
{
	if (srpContext.cullFace == SRP_CULL_FACE_FRONT_AND_BACK)
		return;

	size_t triangleCount;
	SRPTriangle* triangles;
	bool success = assembleTriangles(
		ib, vb, fb, sp, primitive, startIndex, count,
		&triangleCount, &triangles
	);
	if (!success)
		return;

	void* interpolatedBuffer = arenaAlloc(srpContext.arena, sp->vs->nBytesPerOutputVariables);
	for (size_t i = 0; i < triangleCount; i++)
		rasterizeTriangle(&triangles[i], fb, sp, interpolatedBuffer);

	arenaReset(srpContext.arena);
}

static void drawLines(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
)
{
	srpMessageCallbackHelper(
		SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
		"Lines are not implemented yet"
	);
}

static void drawPoints(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
)
{
	srpMessageCallbackHelper(
		SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
		"Points are not implemented yet"
	);
}

static bool isPrimitiveTriangle(SRPPrimitive primitive)
{
	return primitive == SRP_PRIM_TRIANGLES ||
		primitive == SRP_PRIM_TRIANGLE_STRIP ||
		primitive == SRP_PRIM_TRIANGLE_FAN;
}

static bool isPrimitiveLine(SRPPrimitive primitive)
{
	return primitive == SRP_PRIM_LINES ||
		primitive == SRP_PRIM_LINE_STRIP ||
		primitive == SRP_PRIM_LINE_LOOP;
}

static bool isPrimitivePoint(SRPPrimitive primitive)
{
	return primitive == SRP_PRIM_POINTS;
}

SRPVertexBuffer* srpNewVertexBuffer()
{
	SRPVertexBuffer* this = SRP_MALLOC(sizeof(SRPVertexBuffer));
	this->nBytesPerVertex = 0;
	this->nVertices = 0;
	this->nBytesAllocated = 0;
	this->data = NULL;
	return this;
}

void srpVertexBufferCopyData
	(SRPVertexBuffer* this, size_t nBytesPerVertex, size_t nBytesData, const void* data)
{
	// Reallocate the buffer if there is not enough allocated space
	if (nBytesData > this->nBytesAllocated)
	{
		SRP_FREE(this->data);
		this->data = SRP_MALLOC(nBytesData);
		this->nBytesAllocated = nBytesData;
	}

	this->nBytesPerVertex = nBytesPerVertex;
	this->nVertices = nBytesData / nBytesPerVertex;

	memcpy(this->data, data, nBytesData);
}

void srpFreeVertexBuffer(SRPVertexBuffer* this)
{
	SRP_FREE(this->data);
	SRP_FREE(this);
}

void srpDrawVertexBuffer(
	const SRPVertexBuffer* this, const SRPFramebuffer* fb, const SRPShaderProgram* sp,
	SRPPrimitive primitive, size_t startIndex, size_t count
)
{
	drawBuffer(NULL, this, fb, sp, primitive, startIndex, count);
}

static SRPVertex* indexVertexBuffer(const SRPVertexBuffer* this, size_t index)
{
	return (SRPVertex*) INDEX_VOID_PTR(this->data, index, this->nBytesPerVertex);
}

SRPIndexBuffer* srpNewIndexBuffer()
{
	SRPIndexBuffer* this = SRP_MALLOC(sizeof(SRPIndexBuffer));
	this->indicesType = TYPE_UINT8;  // Default value, will be overwritten in `srpIndexBufferCopyData`
	this->nBytesPerIndex = srpSizeofType(this->indicesType);
	this->nIndices = 0;
	this->nBytesAllocated = 0;
	this->data = NULL;
	return this;
}

void srpIndexBufferCopyData(
	SRPIndexBuffer* this, SRPType indicesType, size_t nBytesData, const void* data
)
{
	// Reallocate the buffer if there is not enough allocated space
	if (nBytesData > this->nBytesAllocated)
	{
		SRP_FREE(this->data);
		this->data = SRP_MALLOC(nBytesData);
		this->nBytesAllocated = nBytesData;
	}

	this->indicesType = indicesType;
	this->nBytesPerIndex = srpSizeofType(indicesType);
	this->nIndices = nBytesData / this->nBytesPerIndex;

	memcpy(this->data, data, nBytesData);
}

void srpFreeIndexBuffer(SRPIndexBuffer* this)
{
	SRP_FREE(this->data);
	SRP_FREE(this);
}

static uint64_t indexIndexBuffer(const SRPIndexBuffer* this, size_t index)
{
	void* pIndex = INDEX_VOID_PTR(this->data, index, this->nBytesPerIndex);
	uint64_t ret;
	switch (this->indicesType)
	{
		case TYPE_UINT8:
			ret = (uint64_t) (*(uint8_t*) pIndex);
			break;
		case TYPE_UINT16:
			ret = (uint64_t) (*(uint16_t*) pIndex);
			break;
		case TYPE_UINT32:
			ret = (uint64_t) (*(uint32_t*) pIndex);
			break;
		case TYPE_UINT64:
			ret = (uint64_t) (*(uint64_t*) pIndex);
			break;
		default:
			srpMessageCallbackHelper(
				SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
				"Unexpected type (%i)", this->indicesType
			);
			ret = 0;
	}
	return ret;
}

// @brief Draw an index buffer with specified primitive mode
void srpDrawIndexBuffer(
	const SRPIndexBuffer* this, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
)
{
	drawBuffer(this, vb, fb, sp, primitive, startIndex, count);
}

