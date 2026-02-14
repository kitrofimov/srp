// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Primitive assembly implementation */

#include <assert.h>
#include "primitive_assembly.h"
#include "message_callback_p.h"
#include "context.h"
#include "arena_p.h"
#include "utils.h"

/** Assemble a single triangle.
 *  @param[in] triangleIdx Index of the primitive to assemble, starting from 0,
 * 						   including culled triangles
 *  @param[in] pVarying Pointer to the buffer where vertex shader outputs will be stored
 *  @param[out] tri Pointer to the triangle where the assembled triangle will be stored
 *  @returns `true` if successful, `false` otherwise */
static bool assembleOneTriangle(
    const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
    const SRPFramebuffer* fb, SRPPrimitive prim, size_t startIndex, size_t triangleIdx,
    SRPTriangle* tri, void* pVarying
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

bool assembleTriangles(
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
	for (size_t k = 0; k < nTriangles; k += 1)
	{
		SRPTriangle* tri = &triangles[primitiveID];
		bool success = assembleOneTriangle(
			ib, vb, sp, fb, prim, startIndex, k, tri, pVarying
		);

		if (!success)
			continue;

		tri->id = primitiveID;
		primitiveID++;
	}

	*outTriangleCount = primitiveID;  // Amount of non-culled triangles
	*outTriangles = triangles;
	return true;
}

static bool assembleOneTriangle(
    const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPShaderProgram* sp,
    const SRPFramebuffer* fb, SRPPrimitive prim, size_t startIndex, size_t triangleIdx,
    SRPTriangle* tri, void* varyingBuffer
)
{
    size_t streamIndices[3];
    resolveTriangleTopology(startIndex, triangleIdx, prim, streamIndices);
	const size_t stride = sp->vs->nBytesPerOutputVariables;

    for (uint8_t i = 0; i < 3; i++)
    {
        size_t vertexIndex = (ib) ? indexIndexBuffer(ib, streamIndices[i]) : streamIndices[i];
        SRPVertex* pVertex = indexVertexBuffer(vb, vertexIndex);

        SRPvsInput vsIn = {
            .vertexID = vertexIndex,
            .pVertex  = pVertex,
            .uniform  = sp->uniform
        };

		void* pVarying = INDEX_VOID_PTR(varyingBuffer, 3 * triangleIdx + i, stride);
        tri->v[i] = (SRPvsOutput){
            .position = {0},
            .pOutputVariables = pVarying
        };

        sp->vs->shader(&vsIn, &tri->v[i]);

        // Perspective divide
        double invW = 1.0 / tri->v[i].position[3];
        tri->v[i].position[0] *= invW;
        tri->v[i].position[1] *= invW;
        tri->v[i].position[2] *= invW;
        tri->v[i].position[3] = 1.0;
    }

	bool success = setupTriangle(tri, fb);
    return success;
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
