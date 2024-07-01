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

static void drawBuffer(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
)
{
	const bool isDrawingIndexBuffer = (ib != NULL);

	if (primitive != SRP_PRIM_TRIANGLES)
	{
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Only triangles are implemented"
		);
		return;
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
		return;
	}

	// Allocate memory for three vertex shader output variables (triangle = 3 vertices)
	SRPVertexVariable* outputVertexVariables = SRP_MALLOC(sp->vs->nBytesPerOutputVariables * 3);
	size_t primitiveID = 0;

	for (size_t i = startIndex; i <= endIndex; i += 3)
	{
		SRPvsInput vsIn[3];
		SRPvsOutput vsOut[3];
		for (uint8_t j = 0; j < 3; j++)
		{
			size_t vertexIndex;
			if (isDrawingIndexBuffer)
				vertexIndex = indexIndexBuffer(ib, i+j);
			else
				vertexIndex = i+j;
			SRPVertex* pVertex = indexVertexBuffer(vb, vertexIndex);
			SRPVertexVariable* pOutputVertexVariables = (SRPVertexVariable*) \
				INDEX_VOID_PTR(outputVertexVariables, j, sp->vs->nBytesPerOutputVariables);

			vsIn[j] = (SRPvsInput) {
				.vertexID = i+j,
				.pVertex = pVertex,
				.uniform = sp->uniform
			};
			vsOut[j] = (SRPvsOutput) {
				.position = {0},
				.pOutputVariables = pOutputVertexVariables
			};

			sp->vs->shader(&vsIn[j], &vsOut[j]);

			// Perspective divide
			// See https://stackoverflow.com/questions/10389040/what-does-the-1-w-coordinate-stand-for-in-gl-fragcoord
			vsOut[j].position[0] = vsOut[j].position[0] / vsOut[j].position[3];
			vsOut[j].position[1] = vsOut[j].position[1] / vsOut[j].position[3];
			vsOut[j].position[2] = vsOut[j].position[2] / vsOut[j].position[3];
			vsOut[j].position[3] =                   1. / vsOut[j].position[3];
		}
		drawTriangle(fb, vsOut, sp, primitiveID);
		primitiveID++;
	}

	SRP_FREE(outputVertexVariables);
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
	this->indicesType = TYPE_UINT8;
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

