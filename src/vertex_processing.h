// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Vertex processing functions */

#pragma once

#include "shaders.h"
#include "buffer_p.h"

/** Represents an entry in VertexCache */
typedef struct VertexCacheEntry {
	bool valid;
	SRPvsOutput data;
	double invW;
} VertexCacheEntry;

/** Post-VS cache */
typedef struct VertexCache {
	VertexCacheEntry* entries;
	size_t baseVertex;
	size_t size;
} VertexCache;

/** Allocate post-VS vertex cache */
void allocateVertexCache(
	VertexCache* cache, const SRPIndexBuffer* ib, size_t startIndex, size_t vertexCount
);

/** Fetch vertex shader output from post-VS cache. If not found, compute and store it
 * 	@see assembleOneTriangle() for documentation on other parameters */
VertexCacheEntry* vertexCacheFetch(
	VertexCache* cache, size_t vertexIndex, void* varyingBuffer,
	const SRPVertexBuffer* vb, const SRPShaderProgram* sp
);

/** Run vertex shader and perform perspective divide */
void processVertex(
	size_t vertexIndex, size_t varyingIndex, void* varyingBuffer,
	const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
	SRPvsOutput* outV, double* outInvW
);
