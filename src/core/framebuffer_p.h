// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 * 	@ingroup Framebuffer_internal
 *  Private header for `include/srp/framebuffer.h` */

#pragma once

#include <stdbool.h>
#include "srp/framebuffer.h"
#include "utils/defines.h"

/** @ingroup Framebuffer_internal
 *  @{ */

/** Get pointers inside color and depth buffers for pixel at (x, y)
 *  @param[in] this Pointer to the SRPFramebuffer
 *  @param[in] x,y Coordinates of the needed pixel
 *  @param[out] pColor Pointer to the color value
 *  @param[out] pDepth Pointer to the depth value
 *  @param[out] pStencil Pointer to the stencil value */
void framebufferGetPointers(
	const SRPFramebuffer* this, size_t x, size_t y,
	uint32_t** pColor, float** pDepth, uint8_t** pStencil
);

/** Convert Normalized Device Coordinates to screen-space coordiantes
 *  @param[in] this The pointer to SRPFramebuffer, as returned
 *                    from srpNewFramebuffer
 *  @param[in] NDC Pointer to 3-element float array containing NDC position
 *  @param[out] SS Pointer to 3-element float array that will contain SS
 *                 position after the call. The z-component is the same as
 *                 z-component of NDC coordinates. */
void framebufferNDCToScreenSpace
	(const SRPFramebuffer* this, const float* NDC, float* SS);

/** @} */  // ingroup Framebuffer_internal
