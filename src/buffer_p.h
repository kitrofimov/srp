// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

/** @file
 *  Private header for `include/buffer.h` */

#include "buffer.h"

/** @ingroup Buffer_internal
 *  @{ */

struct SRPVertexBuffer
{
	size_t nBytesPerVertex;  /**< How many bytes does one vertex occupy */
	size_t nVertices;        /**< How many vertices does this vertex buffer contain */
	size_t nBytesAllocated;  /**< How many bytes was already allocated for `data` */
	SRPVertex* data;         /**< Pointer to the vertex data */
};

struct SRPIndexBuffer
{
	SRPType indicesType;     /**< Type of stored indices @see srpIndexBufferCopyData */
	size_t nBytesPerIndex;   /**< How many bytes does one index occupy */
	size_t nIndices;         /**< How many indices does this index buffer contain */
	size_t nBytesAllocated;  /**< How many bytes was already allocated for `data` */
	void* data;              /**< Pointer to the index data */
};

/** Get an element stored in SRPIndexBuffer.
 *  Needed because SRPIndexBuffer stores opaque index types.
 *  @param[in] this Pointer to SRPIndexBuffer
 *  @param[in] ibIndex Index of the element in the SRPIndexBuffer
 *  @return Element upcasted to `uint64_t` */
uint64_t indexIndexBuffer(const SRPIndexBuffer* this, size_t ibIndex);

/** Get a vertex stored in SRPVertexBuffer.
 *  @param[in] this Pointer to SRPVertexBuffer
 *  @param[in] index Index of the vertex to get
 *  @return Requested vertex */
SRPVertex* indexVertexBuffer(const SRPVertexBuffer* this, size_t index);

/** @} */  // ingroup Buffer_internal
