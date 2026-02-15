// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Point rasterization */

#include "framebuffer_p.h"
#include "shaders.h"
#include "vec.h"

typedef struct SRPPoint {
	SRPvsOutput v;  /**< Output of the vertex shader */
	size_t id;      /**< ID of the primitive, starting from 0 */
} SRPPoint;

/** Rasterize a point
 *  @param[in] point Pointer to the point to draw
 *  @param[in] fb The framebuffer to draw to
 *  @param[in] sp The shader program to use */
void rasterizePoint(
	SRPPoint* point, const SRPFramebuffer* fb,
	const SRPShaderProgram* restrict sp
);
