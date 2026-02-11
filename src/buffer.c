// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <stdio.h>
#include <string.h>
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
 *  @param[in] `ib` Pointer to index buffer, or `NULL` if assembling from vertex buffer
 *  @param[in] `vb` Pointer to vertex buffer
 *  @param[in] `fb` Pointer to the framebuffer to draw to (needed for NDC to screen-space conversion)
 *  @param[in] `sp` Pointer to the shader program to use
 *  @param[in] `primitive` Primitive type (e.g. SRP_PRIM_TRIANGLES)
 *  @param[in] `startIndex` Index of the first vertex/index to assemble
 *  @param[in] `count` Number of vertices/indices to assemble
 *  @param[out] `outTriangleCount` Number of triangles assembled
 *  @param[out] `outTriangles` Pointer to the array of assembled triangles.
 * 				Must be freed by the caller using cleanUpTriangles()
 *  @returns `true` if successful, `false` otherwise. If `false` is returned,
 * 			 `*outTriangleCount` and `*outTriangles` are undefined. You shouldn't
 * 			 call cleanUpTriangles() in this case. */
static bool assembleTriangles(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count,
	size_t* outTriangleCount, SRPTriangle** outTriangles
);

/** Free the memory allocated for triangles assembled by assembleTriangles()
 *  @param[in] triangleCount Number of triangles in the array
 *  @param[in] triangles Pointer to the array of triangles returned by assembleTriangles() */
static void cleanUpTriangles(size_t triangleCount, SRPTriangle* triangles);

/** Draw either SRPIndexBuffer or SRPVertexBuffer.
 *  If `ib == NULL`, draws the vertex buffer, else draws index buffer.
 *  Created because vertex and index buffer drawing are very similar,
 *  with an intent to avoid code duplication
 *  @see srpDrawVertexBuffer() srpDrawIndexBuffer() for parameter documentation */
static void drawBuffer(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
);

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
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count,
	size_t* outTriangleCount, SRPTriangle** outTriangles
)
{
	const bool isDrawingIndexBuffer = (ib != NULL);

	if (primitive != SRP_PRIM_TRIANGLES)
	{
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Only triangles are implemented"
		);
		return false;
	}

	if (count % 3 != 0)
	{
		srpMessageCallbackHelper(
			SRP_MESSAGE_WARNING, SRP_MESSAGE_SEVERITY_LOW, __func__,
			"Vertex count not divisible by 3. The last %i vertex/vertices will be ignored", count % 3
		);
	}

	// Index of the last vertex that will be touched
	// That is, only vertices with indices [startIndex, endIndex] will be drawn
	const size_t endIndex = startIndex + count - 1;
	const size_t bufferSize = (isDrawingIndexBuffer) ? ib->nIndices : vb->nVertices;
	if (endIndex >= bufferSize)
	{
		const char* errorMessage = (isDrawingIndexBuffer) ? \
			"Attempt to OOB access index buffer (read) at indices %i-%i (size: %i)\n" : \
			"Attempt to OOB access vertex buffer (read) at indices %i-%i (size: %i)\n";
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			errorMessage, startIndex, endIndex, bufferSize
		);
		return false;
	}

	size_t triangleCount = count / 3;
	SRPTriangle* triangles = SRP_MALLOC(sizeof(SRPTriangle) * triangleCount);
	size_t primitiveID = 0;

	// Arena allocation for varying variables
	void* varyingBuffer = SRP_MALLOC(sp->vs->nBytesPerOutputVariables * 3 * triangleCount);
	SRPVertexVariable* pVarying = varyingBuffer;

	for (size_t i = startIndex; i <= endIndex; i += 3)
	{
		SRPTriangle* tri = &triangles[primitiveID];

		for (uint8_t j = 0; j < 3; j++)
		{
			size_t vertexIndex = isDrawingIndexBuffer ? indexIndexBuffer(ib, i+j) : (i+j);
			SRPVertex* pVertex = indexVertexBuffer(vb, vertexIndex);

			SRPvsInput vsIn = {
				.vertexID = vertexIndex,
				.pVertex  = pVertex,
				.uniform  = sp->uniform
			};
			tri->v[j] = (SRPvsOutput) {
				.position = {0},
				.pOutputVariables = pVarying,
			};

			// Increment the arena allocation pointer
			pVarying = (void*) INDEX_VOID_PTR(pVarying, 1, sp->vs->nBytesPerOutputVariables);

			sp->vs->shader(&vsIn, &tri->v[j]);

			// Perspective divide
			tri->v[j].position[0] /= tri->v[j].position[3];
			tri->v[j].position[1] /= tri->v[j].position[3];
			tri->v[j].position[2] /= tri->v[j].position[3];
			tri->v[j].position[3] = 1.0f;
		}

		setupTriangle(tri, fb);

		tri->id = primitiveID;
		primitiveID++;
	}

	*outTriangleCount = triangleCount;
	*outTriangles = triangles;
	return true;
}

static void cleanUpTriangles(size_t triangleCount, SRPTriangle* triangles)
{
	SRP_FREE(triangles);
}

static void drawBuffer(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
)
{
	size_t triangleCount;
	SRPTriangle* triangles;
	bool success = assembleTriangles(
		ib, vb, fb, sp, primitive, startIndex, count,
		&triangleCount, &triangles
	);
	if (!success)
		return;

	for (size_t i = 0; i < triangleCount; i++)
		rasterizeTriangle(&triangles[i], fb, sp);

	cleanUpTriangles(triangleCount, triangles);
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

