// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

/** @file
 *  Triangle rasterization functions */

#include "framebuffer.h"
#include "shaders.h"
#include "vec.h"

/** @ingroup Rasterization
 *  @{ */

/** Represents a triangle and stores data needed for its rasterization */
typedef struct SRPTriangle {
	SRPvsOutput v[3];      /**< Output of the vertex shader */
	vec3d* p_ndc[3];       /**< Pointer to vertices' positions in NDC */
	vec3d ss[3];	       /**< Vertices' positions in screen-space */
	vec3d edge[3];         /**< Screen space edge vectors */
	bool edgeTL[3];        /**< Whether or not the edge is flat top or left */
	vec2d minBP;           /**< Minimum bounding point (screen-space) */
	vec2d maxBP;           /**< Maximum bounding point (screen-space) */
	double lambda[3];      /**< Barycentric coordinates at the current pixel */
	double lambda_row[3];  /**< Barycentric coordinates at the beginning of the row */
	double dldx[3];        /**< Barycentric coordinates' delta values for +X movement */
	double dldy[3];        /**< Barycentric coordinates' delta values for +Y movement */
	double invZ[3];        /**< 1 / Z, where Z is in [0; 1]. Needed for perspective-correct interpolation */
	size_t id;             /**< ID of the primitive, starting from 0 */
} SRPTriangle;

/** Calculate internal data for triangle rasterization
 *  @param[in] `tri` The triangle to set up. Its `v` field is required to be filled
 *  @param[in] `fb` The framebuffer to use for NDC to screen-space conversion */
void setupTriangle(
	SRPTriangle* tri, const SRPFramebuffer* fb
);

/** Draw the triangle that is specified by three vertices to the framebuffer
 *  @param[in] `fb` The framebuffer to draw to
 *  @param[in] `vertices` Pointer to an array of 3 `SRPvsOutput`s
 *  @param[in] `sp` The shader program to use
 *  @param[in] `primitiveID` Primitive ID of this triangle */
void rasterizeTriangle(
	SRPTriangle* triangle, const SRPFramebuffer* fb,
	const SRPShaderProgram* restrict sp
);

/** @} */  // ingroup Rasterization

