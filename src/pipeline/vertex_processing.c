// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Vertex_processing
 *  Vertex processing & post-VS cache implementation */

#include <assert.h>
#include "pipeline/vertex_processing.h"
#include "memory/arena_p.h"
#include "utils/voidptr.h"
#include "math/utils.h"

/** @ingroup Vertex_processing
 *  @{ */

/** Compute minimal and maximal vertex indices given stream indices */
static void computeMinMaxVI(
	const SRPIndexBuffer* ib, size_t startIndex, size_t vertexCount,
	size_t* outMinVI, size_t* outMaxVI
);

void allocateVertexCache(
	VertexCache* cache, const SRPIndexBuffer* ib, size_t startIndex,
	size_t vertexCount, size_t varyingSize
)
{
	size_t minVI, maxVI;
	computeMinMaxVI(ib, startIndex, vertexCount, &minVI, &maxVI);

	// Setting all cache to zero before using it (calloc)
	cache->baseVertex = minVI;
	cache->size = maxVI - minVI + 1;
	cache->entries = ARENA_CALLOC(sizeof(VertexCacheEntry) * cache->size);
	cache->varyingBlock = ARENA_ALLOC(varyingSize * cache->size);
}

SRPVertexShaderOut* vertexCacheFetch(
	VertexCache* cache, size_t vertexIndex,	const SRPVertexBuffer* vb,
	const SRPShaderProgram* sp
)
{
	size_t idx = vertexIndex - cache->baseVertex;
	VertexCacheEntry* entry = &cache->entries[idx];

	if (!entry->valid)
	{
		processVertex(vertexIndex, cache->varyingBlock, idx, vb, sp, &entry->data);
		entry->valid = true;
	}

	return &entry->data;
}

void processVertex(
	size_t vertexIndex, void* varyingBlock, size_t varyingIndex,
	const SRPVertexBuffer* vb, const SRPShaderProgram* sp, SRPVertexShaderOut* outV
)
{
	SRPVertex* pVertex = indexVertexBuffer(vb, vertexIndex);
	void* pVarying = INDEX_VOID_PTR(varyingBlock, varyingIndex, sp->vs->varyingsSize);

	SRPVertexShaderIn vsIn = {
		.vertexID = vertexIndex,
		.vertex  = pVertex,
		.uniform  = sp->uniform
	};
	*outV = (SRPVertexShaderOut) {
		.position = {0},
		.varyings = pVarying
	};

	sp->vs->shader(&vsIn, outV);
}

void applyPerspectiveDivide(SRPVertexShaderOut* output, float* outInvW)
{
    float clipW = output->position[3];
    float invW = 1.0 / clipW;
	assert(!ROUGHLY_ZERO(invW));
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

/** @} */  // ingroup Vertex_processing
