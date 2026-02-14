// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Primitive assembly */

#include "triangle.h"
#include "shaders.h"
#include "buffer_p.h"

/** Call the vertex shader and assemble triangles from vertex or index buffer.
 *  If `ib == NULL`, assembles from vertex buffer, else from index buffer.
 *  Uses memory from SRPArena, so the returned triangles are valid until the
 *  next call to arenaReset().
 *  @param[in] ib Pointer to index buffer, or `NULL` if assembling from vertex buffer
 *  @param[in] vb Pointer to vertex buffer
 *  @param[in] fb Pointer to the framebuffer to draw to (needed for NDC to screen-space conversion)
 *  @param[in] sp Pointer to the shader program to use
 *  @param[in] primitive Primitive type (e.g. SRP_PRIM_TRIANGLES)
 *  @param[in] startIndex Index of the first vertex/index to assemble
 *  @param[in] count Number of vertices/indices to assemble
 *  @param[out] outTriangleCount Number of triangles assembled
 *  @param[out] outTriangles Pointer to the array of assembled triangles
 *  @returns `true` if successful, `false` otherwise. If `false` is returned,
 * 			 `*outTriangleCount` and `*outTriangles` are undefined */
bool assembleTriangles(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count,
	size_t* outTriangleCount, SRPTriangle** outTriangles
);
