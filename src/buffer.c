// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Buffer implementation */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "buffer_p.h"
#include "draw.h"
#include "message_callback_p.h"
#include "utils.h"
#include "defines.h"

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

SRPVertex* indexVertexBuffer(const SRPVertexBuffer* this, size_t index)
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

uint64_t indexIndexBuffer(const SRPIndexBuffer* this, size_t index)
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

