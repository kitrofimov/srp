// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Private header for `include/framebuffer.h` */

#pragma once

#include <stdbool.h>
#include "srp/framebuffer.h"

/** @ingroup Framebuffer
 *  @{ */

/** Draw a pixel in a framebuffer with specified depth, making no depth checks
 *  @param[in] this The pointer to SRPFramebuffer, as returned 
 *                  from srpNewFramebuffer
 *  @param[in] x,y Position at which to draw the pixel. X-axis points
 *                 from left to right, Y-axis - from top to bottom
 *  @param[in] depth Depth value to assign to the pixel, assumed to be inside
 *                   the [-1, 1] interval
 *  @param[in] color RBGA8888 color to draw. */
void framebufferDrawPixel(
	const SRPFramebuffer* this, size_t x, size_t y, double depth,
	uint32_t color
);

/** Draw a pixel in a framebuffer with specified depth, making no depth checks
 *  @param[in] this The pointer to SRPFramebuffer
 *  @param[in] x,y Position at which to test the depth
 *  @param[in] depth Depth value to test
 *  @return Whether or not the fragment has passed the depth test. */
bool framebufferDepthTest(
	const SRPFramebuffer* this, size_t x, size_t y, double depth
);

/** Convert Normalized Device Coordinates to screen-space coordiantes
 *  @param[in] this The pointer to SRPFramebuffer, as returned
 *                    from srpNewFramebuffer
 *  @param[in] NDC Pointer to 3-element double array containing NDC position
 *  @param[out] SS Pointer to 3-element double array that will contain SS
 *                   position after the call. The z-component is the same as
 *                   z-component of NDC coordinates. */
void framebufferNDCToScreenSpace
	(const SRPFramebuffer* this, const double* NDC, double* SS);

/** @} */  // ingroup Framebuffer

