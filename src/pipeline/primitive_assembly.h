// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Primitive_assembly
 *  Primitive assembly */

#pragma once

#include "raster/triangle.h"
#include "raster/line.h"
#include "raster/point.h"
#include "srp/shaders.h"
#include "core/buffer_p.h"

/** @ingroup Primitive_assembly
 *  @{ */

/** Call the vertex shader and assemble triangles from vertex or index buffer,
 *  possibly converting them to lines or points according to the set polygon mode.
 *  If `ib == NULL`, assembles from vertex buffer, else from index buffer.
 *  Uses memory from SRPArena, so the returned triangles are valid until the
 *  next call to arenaReset().
 *  @param[in] ib Pointer to index buffer, or `NULL` if assembling from vertex buffer
 *  @param[in] vb Pointer to vertex buffer
 *  @param[in] fb Pointer to the framebuffer to draw to (needed for NDC to
 * 				  screen-space conversion)
 *  @param[in] sp Pointer to the shader program to use
 *  @param[in] prim Primitive type (one of SRP_PRIM_TRIANGLES,
 * 					SRP_PRIM_TRIANGLE_STRIP or SRP_PRIM_TRIANGLE_FAN)
 *  @param[in] startIndex First stream index to assemble
 *  @param[in] vertexCount Number of stream indices to assemble
 *  @param[out] outCount Amount of assembled primitives
 *  @param[out] outPrimitives Pointer to the array of assembled primitives
 *  @returns `true` if successful, `false` otherwise. If `false` is returned,
 * 			 `*outCount` and `*outPrimitives` are 0 and NULL */
bool assembleTrianglesGeneric(
    const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
    const SRPShaderProgram* sp, SRPPrimitive prim, size_t startIndex, size_t vertexCount,
    size_t* outCount, void** outPrimitives
);

/** Call the vertex shader and assemble lines from vertex or index buffer.
 *  If `ib == NULL`, assembles from vertex buffer, else from index buffer.
 *  Uses memory from SRPArena, so the returned points are valid until the
 *  next call to arenaReset().
 *  @param[in] ib Pointer to index buffer, or `NULL` if assembling from vertex buffer
 *  @param[in] vb Pointer to vertex buffer
 *  @param[in] fb Pointer to the framebuffer to draw to (needed for NDC to
 * 				  screen-space conversion)
 *  @param[in] sp Pointer to the shader program to use
 *  @param[in] primitive Primitive type (one of SRP_PRIM_LINES, SRP_PRIM_LINE_STRIP
 * 						 or SRP_PRIM_LINE_LOOP)
 *  @param[in] startIndex First stream index to assemble
 *  @param[in] count Number of stream indices to assemble
 *  @param[out] outLineCount Amount of assembled lines
 *  @param[out] outLines Pointer to the array of `count` assembled lines
 *  @return `true` if successful, `false` otherwise. If `false` if returned,
 * 			`*outLineCount` and `*outLines` are undefined */
bool assembleLines(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count,
	size_t* outLineCount, SRPLine** outLines
);

/** Call the vertex shader and assemble points from vertex or index buffer.
 *  If `ib == NULL`, assembles from vertex buffer, else from index buffer.
 *  Uses memory from SRPArena, so the returned points are valid until the
 *  next call to arenaReset().
 *  @param[in] ib Pointer to index buffer, or `NULL` if assembling from vertex buffer
 *  @param[in] vb Pointer to vertex buffer
 *  @param[in] fb Pointer to the framebuffer to draw to (needed for NDC to
 * 				  screen-space conversion)
 *  @param[in] sp Pointer to the shader program to use
 *  @param[in] startIndex First stream index to assemble
 *  @param[in] count Number of stream indices to assemble
 *  @param[out] outPointCount Amount of assembled points
 *  @param[out] outPoints Pointer to the array of `count` assembled points
 *  @return `true` if successful, `false` otherwise. If `false` if returned,
 * 			`*outPointCount` and `*outPoints` are undefined */
bool assemblePoints(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, size_t startIndex, size_t count,
	size_t* outPointCount, SRPPoint** outPoints
);

/** @} */  // ingroup Primitive_assembly
