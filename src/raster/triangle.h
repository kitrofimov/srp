// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Rasterization
 *  Triangle rasterization */

#pragma once

#include "srp/framebuffer.h"
#include "srp/shaders.h"
#include "srp/vec.h"

/** @ingroup Rasterization
 *  @{ */

/** Triangle primitive. Stores data needed for its rasterization */
typedef struct SRPTriangle {
	SRPVertexShaderOut v[3];  /**< Outputs of the vertex shader */
	vec3* p_ndc[3];           /**< Pointers to vertices' positions in NDC */
	vec3 ss[3];	              /**< Vertices' positions in screen-space */
	vec3 edge[3];             /**< Screen space edge vectors */
	bool edgeTL[3];           /**< Whether or not the edge is flat top or left
							       (0th edge: 0th vertex -> 1st vertex, etc.) */
	vec2 minBP;               /**< Minimum bounding point (screen-space) */
	vec2 maxBP;               /**< Maximum bounding point (screen-space) */
	float lambda[3];          /**< Barycentric coordinates at the current pixel */
	float lambda_row[3];      /**< Barycentric coordinates at the beginning of the row */
	float dldx[3];            /**< Barycentric coordinates' delta values for +X movement */
	float dldy[3];            /**< Barycentric coordinates' delta values for +Y movement */
	float invW[3];            /**< 1 / clip-space W. Needed for perspective-correct interpolation */
	bool isFrontFacing;       /**< Whether or not the triangle is front-facing */
	size_t id;                /**< ID of the primitive, starting from 0 */
} SRPTriangle;

/** Setup triangle for rasterization, performing perspective divide and
 *  calculating internal variables
 *  @param[in] tri The triangle to set up. Its `v` field is required to be filled
 *  @param[in] fb The framebuffer to use for NDC to screen-space conversion
 *  @return `false` if it is culled and should not be rasterized, `true` otherwise */
bool setupTriangle(SRPTriangle* tri, const SRPFramebuffer* fb);

/** Rasterize a triangle
 *  @param[in] triangle Pointer to the triangle to draw
 *  @param[in] fb The framebuffer to draw to
 *  @param[in] sp The shader program to use
 *  @param[in] interpolatedBuffer Pointer to a temporary buffer where varyings 
 * 			   for each fragment will be stored. Must be big enough to hold all
 * 			   interpolated attributes for ONE vertex. */
void rasterizeTriangle(
	SRPTriangle* triangle, const SRPFramebuffer* fb,
	const SRPShaderProgram* restrict sp, void* interpolatedBuffer
);

/** @} */  // ingroup Rasterization
