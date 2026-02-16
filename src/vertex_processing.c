// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Vertex processing functions implementation */

#include "vertex_processing.h"
#include "arena_p.h"
#include "utils.h"

/** Compute minimal and maximal vertex indices given stream indices */
static void computeMinMaxVI(
	const SRPIndexBuffer* ib, size_t startIndex, size_t vertexCount,
	size_t* outMinVI, size_t* outMaxVI
);

/** Apply perspective divide to the output of the vertex shader,
 *  saving the 1 / W_clip value
 *  @param[in] output Output of the vertex shader
 *  @param[out] outInvW Pointer where 1/W value will be stored. May be NULL */
static void applyPerspectiveDivide(SRPvsOutput* output, double* outInvW);

void allocateVertexCache(
	VertexCache* cache, const SRPIndexBuffer* ib, size_t startIndex, size_t vertexCount
)
{
	size_t minVI, maxVI;
	computeMinMaxVI(ib, startIndex, vertexCount, &minVI, &maxVI);

	// Setting all cache to zero before using it (calloc)
	cache->baseVertex = minVI;
	cache->size = maxVI - minVI + 1;
	cache->entries = ARENA_CALLOC(sizeof(VertexCacheEntry) * cache->size);
}

VertexCacheEntry* vertexCacheFetch(
	VertexCache* cache, size_t vertexIndex,	void* varyingBuffer,
	const SRPVertexBuffer* vb, const SRPShaderProgram* sp
)
{
	VertexCacheEntry* entry = &cache->entries[vertexIndex - cache->baseVertex];

	if (!entry->valid)
	{
		processVertex(
			vertexIndex, vertexIndex - cache->baseVertex, varyingBuffer, vb, sp,
			&entry->data, &entry->invW
		);
		entry->valid = true;
	}

	return entry;
}

void processVertex(
	size_t vertexIndex, size_t varyingIndex, void* varyingBuffer,
	const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
	SRPvsOutput* outV, double* outInvW
)
{
	SRPVertex* pVertex = indexVertexBuffer(vb, vertexIndex);
	const size_t stride = sp->vs->nBytesPerOutputVariables;
	void* pVarying = INDEX_VOID_PTR(varyingBuffer, varyingIndex, stride);

	SRPvsInput vsIn = {
		.vertexID = vertexIndex,
		.pVertex  = pVertex,
		.uniform  = sp->uniform
	};
	*outV = (SRPvsOutput) {
		.position = {0},
		.pOutputVariables = pVarying
	};

	sp->vs->shader(&vsIn, outV);
	applyPerspectiveDivide(outV, outInvW);
}

static void applyPerspectiveDivide(SRPvsOutput* output, double* outInvW)
{
    double clipW = output->position[3];
    double invW = 1.0 / clipW;
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
