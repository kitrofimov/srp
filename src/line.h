// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Line rasterization */

#include "framebuffer_p.h"
#include "shaders.h"

typedef struct SRPLine {
	SRPvsOutput v[2];  /**< Output of the vertex shader */
	size_t id;         /**< ID of the primitive, starting from 0 */
} SRPLine;

/** Rasterize a point
 *  @param[in] line Pointer to the line to draw
 *  @param[in] fb The framebuffer to draw to
 *  @param[in] sp The shader program to use */
void rasterizeLine(
	SRPLine* line, const SRPFramebuffer* fb,
	const SRPShaderProgram* restrict sp
);
