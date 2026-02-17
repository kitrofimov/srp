// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Point rasterization */

#include "core/framebuffer_p.h"
#include "srp/shaders.h"
#include "srp/vec.h"

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

/** Perform perspective divide on a point
 *  @param[in] p Point to setup */
void setupPoint(SRPPoint* p);
