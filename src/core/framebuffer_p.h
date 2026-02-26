// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Private header for `include/framebuffer.h` */

#pragma once

#include <stdbool.h>
#include "srp/framebuffer.h"
#include "utils/defines.h"

/** @ingroup Framebuffer
 *  @{ */

void framebufferGetColorAndDepthPointers(
	const SRPFramebuffer* this, size_t x, size_t y,
	uint32_t** pColor, float** pDepth
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

/** @} */  // ingroup Framebuffer

