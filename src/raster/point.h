// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Rasterization
 *  Point rasterization */

#pragma once

#include "core/framebuffer_p.h"
#include "srp/shaders.h"
#include "srp/vec.h"

/** @ingroup Rasterization
 *  @{ */

/** Point primitive. Stores data needed for its rasterization */
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

/** Perform perspective divide on a point. Created to be structurally similar
 *  to setupTriangle(), setupLine()
 *  @param[in] p Point to setup */
void setupPoint(SRPPoint* p);

/** @} */  // ingroup Rasterization
