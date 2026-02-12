// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Buffer objects (SRPVertexBuffer and SRPIndexBuffer) */

#pragma once

#include "framebuffer.h"
#include "shaders.h"

/** @ingroup Buffer
 *  @{ */

/** Specifies primitives that can be drawn
 *  @see srpDrawVertexBuffer() srpDrawIndexBuffer() */
typedef enum SRPPrimitive
{
	SRP_PRIM_POINTS,          /**< Points (0, 1, 2, 3, ..., n-1) */
	SRP_PRIM_LINES,           /**< Lines (0-1, 2-3, ...) If non-even number of
								   vertices, then the extra one is ignored) */
	SRP_PRIM_LINE_STRIP,      /**< Lines (0-1, 1-2, ..., {n-2}-{n-1}) */
	SRP_PRIM_LINE_LOOP,       /**< Lines (0-1, 1-2, ..., {n-2}-{n-1}, {n-1}-0) */
	SRP_PRIM_TRIANGLES,       /**< Triangles (0-1-2, 3-4-5, ...) */
	SRP_PRIM_TRIANGLE_STRIP,  /**< Triangles (0-1-2, 1-2-3, 2-3-4, ...) */
	SRP_PRIM_TRIANGLE_FAN     /**< Triangles (0-1-2, 0-2-3, 0-3-4, ...) */
} SRPPrimitive;

/** Stores vertex data, similarly to VBO in OpenGL */
typedef struct SRPVertexBuffer SRPVertexBuffer;

/** Stores indices to vertices from SRPVertexBuffer similarly to EBO in OpenGL */
typedef struct SRPIndexBuffer SRPIndexBuffer;

/** Construct the vertex buffer. @see srpVertexBufferCopyData()
 *  @return A pointer to constructed vertex buffer */
SRPVertexBuffer* srpNewVertexBuffer();

/** Free the vertex buffer
 *  @param[in] this The pointer to vertex buffer, as returned from srpNewVertexBuffer() */
void srpFreeVertexBuffer(SRPVertexBuffer* this);

/** Copy the vertex data over to vertex buffer
 *  @param[in] this The pointer to vertex buffer
 *  @param[in] nBytesPerVertex The size of one vertex, in bytes
 *  @param[in] nBytesData The size of vertex data, in bytes
 *  @param[in] data The pointer to vertex data */
void srpVertexBufferCopyData
	(SRPVertexBuffer* this, size_t nBytesPerVertex, size_t nBytesData, const void* data);

/** Draw vertices from vertex buffer
 *  @param[in] this The pointer to vertex buffer to read the vertex data from
 *  @param[in] fb The framebuffer to draw to
 *  @param[in] sp The shader program to use
 *  @param[in] primitive Specifies the primitive to draw
 *  @param[in] startIndex Specifies from what index to start drawing
 *  @param[in] count Specifies how many vertices to draw */
void srpDrawVertexBuffer(
	const SRPVertexBuffer* this, const SRPFramebuffer* fb, const SRPShaderProgram* sp,
	SRPPrimitive primitive, size_t startIndex, size_t count
);

/** Construct the index buffer
 *  @return A pointer to constructed index buffer */
SRPIndexBuffer* srpNewIndexBuffer();

/** Copy the vertex data over to vertex buffer
 *  @param[in] this The pointer to index buffer
 *  @param[in] indicesType The type of indices passed by data.
 *             Must be one of TYPE_UINT8, TYPE_UINT16, TYPE_UINT32, TYPE_UIN64
 *  @param[in] nBytesData The size of index data 
 *  @param[in] data The pointer to an array of indices of type indicesType */
void srpIndexBufferCopyData
	(SRPIndexBuffer* this, SRPType indicesType, size_t nBytesData, const void* data);

/** Free the index buffer
 *  @param[in] this The pointer to index buffer, as returned from srpNewIndexBuffer() */
void srpFreeIndexBuffer(SRPIndexBuffer* this);

/** Draw vertices using both vertex and index buffers
 *  @param[in] this The index buffer to read the indices from
 *  @param[in] vb The vertex buffer to read the vertex data from
 *  @param[in] fb The framebuffer to draw to
 *  @param[in] sp The shader program to use
 *  @param[in] primitive Specifies the primitive to draw
 *  @param[in] startIndex Specifies from what index buffer's index to start drawing
 *  @param[in] count Specifies how many indices to draw */
void srpDrawIndexBuffer(
	const SRPIndexBuffer* this, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
);

/** @} */  // ingroup Buffer

