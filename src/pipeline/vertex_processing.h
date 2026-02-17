// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Vertex processing functions */

#pragma once

#include "srp/shaders.h"
#include "core/buffer_p.h"

/** Represents an entry in VertexCache */
typedef struct VertexCacheEntry {
	bool valid;
	SRPvsOutput data;
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

/** Fetch vertex shader output from post-VS cache. If not found, compute and store it.
 *  Returns clip-space positions (does not perform perspective divide)
 * 	@see assembleOneTriangle() for documentation on other parameters */
SRPvsOutput* vertexCacheFetch(
	VertexCache* cache, size_t vertexIndex, void* varyingBuffer,
	const SRPVertexBuffer* vb, const SRPShaderProgram* sp
);

/** Run vertex shader */
void processVertex(
	size_t vertexIndex, size_t varyingIndex, void* varyingBuffer,
	const SRPVertexBuffer* vb, const SRPShaderProgram* sp, SRPvsOutput* outV
);

/** Apply perspective divide to the output of the vertex shader,
 *  saving the 1 / W_clip value
 *  @param[in] output Output of the vertex shader
 *  @param[out] outInvW Pointer where 1/W value will be stored. May be NULL */
void applyPerspectiveDivide(SRPvsOutput* output, double* outInvW);
