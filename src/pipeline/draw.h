// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Draw dispatch */

#include "core/buffer_p.h"

/** Draw either SRPIndexBuffer or SRPVertexBuffer.
 *  If `ib == NULL`, draws the vertex buffer, else draws index buffer.
 *  Created because vertex and index buffer drawing are very similar,
 *  with an intent to avoid code duplication
 *  @see srpDrawVertexBuffer() srpDrawIndexBuffer() for parameter documentation */
void drawBuffer(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
);
