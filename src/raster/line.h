// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Line rasterization */

#pragma once

#include "core/framebuffer_p.h"
#include "srp/shaders.h"
#include "srp/vec.h"

typedef struct SRPLine {
	SRPvsOutput v[2];  /**< Output of the vertex shader */
	vec3d ss[2];	   /**< Vertices' positions in screen-space */
	double invW[2];    /**< 1 / clip-space W. Needed for perspective-correct interpolation */
	size_t id;         /**< ID of the primitive, starting from 0 */
} SRPLine;

/** Rasterize a point
 *  @param[in] line Pointer to the line to draw
 *  @param[in] fb The framebuffer to draw to
 *  @param[in] sp The shader program to use
 *  @param[in] interpolatedBuffer Pointer to a temporary buffer where varyings 
 * 			   for each fragment will be stored. Must be big enough to hold all
 * 			   interpolated attributes for ONE vertex. */
void rasterizeLine(
	SRPLine* line, const SRPFramebuffer* fb,
	const SRPShaderProgram* restrict sp, void* interpolatedBuffer
);

/** Calculate internal data for line rasterization
 *  @param[in] line The line to set up. Its `v` field is required to be filled
 *  @param[in] fb The framebuffer to use for NDC to screen-space conversion */
void setupLine(SRPLine* line, const SRPFramebuffer* fb);
