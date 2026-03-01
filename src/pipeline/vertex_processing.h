// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Vertex_processing
 *  Vertex processing & post-VS cache */

#pragma once

#include "srp/shaders.h"
#include "core/buffer_p.h"

/** @ingroup Vertex_processing
 *  @{ */

/** Represents an entry in VertexCache */
typedef struct VertexCacheEntry {
	bool valid;               /**< Whether or not this entry is valid */
	SRPVertexShaderOut data;  /**< The vertex transformed by the vertex shader */
} VertexCacheEntry;

/** Post-VS cache */
typedef struct VertexCache {
	VertexCacheEntry* entries;  /**< The array of vertex cache entries */
	size_t baseVertex;          /**< The base vertex of the cache, all other vertices
									 index the `entries` array relative to it. */
	size_t size;                /**< Cache size */
	void* varyingBlock;         /**< Pointer to the block of memory where varyings
									 of vertex-cache-stored vertices will be */
} VertexCache;

/** Initialize / allocate vertex cache
 *  @param[in] cache Pointer to the cache
 *  @param[in] ib The SRPIndexBuffer being used
 *  @param[in] startIndex First stream index
 *  @param[in] vertexCount How many vertices from the `ib` you want to process
 *  @param[in] varyingSize The size of varying vertex parameters, in bytes */
void allocateVertexCache(
	VertexCache* cache, const SRPIndexBuffer* ib, size_t startIndex,
	size_t vertexCount, size_t varyingSize
);

/** Fetch vertex shader output from post-VS cache. If not found, compute and store it.
 *  Returns clip-space positions (does not perform perspective divide)
 * 	@param[in] cache The vertex cache
 * 	@param[in] vertexIndex The index of the vertex to fetch (not its stream index!)
 * 	@param[in] vb The SRPVertexBuffer being used
 * 	@param[in] sp The SRPShaderProgram being used
 *  @returns The processed, post-VS vertex */
SRPVertexShaderOut* vertexCacheFetch(
	VertexCache* cache, size_t vertexIndex,	const SRPVertexBuffer* vb,
	const SRPShaderProgram* sp
);

/** Run vertex shader
 *  @param[in] vertexIndex The index of the vertex to process (not its stream index!)
 *  @param[in] varyingBlock Pointer to the block where vertex varyings are stored
 *  @param[in] varyingIndex Index into the `varyingBlock`
 *  @param[in] vb The SRPVertexBuffer being used
 *  @param[in] sp The SRPShaderProgram being used
 *  @param[out] outV Where to store the processed vertex */
void processVertex(
	size_t vertexIndex, void* varyingBlock, size_t varyingIndex,
	const SRPVertexBuffer* vb, const SRPShaderProgram* sp, SRPVertexShaderOut* outV
);

/** Apply perspective divide to the output of the vertex shader,
 *  optionally saving the 1 / W_clip value
 *  @param[in] output Output of the vertex shader
 *  @param[out] outInvW Pointer where 1/W value will be stored. May be NULL */
void applyPerspectiveDivide(SRPVertexShaderOut* output, float* outInvW);

/** @} */  // ingroup Vertex_processing
