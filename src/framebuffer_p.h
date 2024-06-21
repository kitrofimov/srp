// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Private header for `include/framebuffer.h` */

#pragma once

#include "framebuffer.h"

/** @ingroup Framebuffer
 *  @{ */

/** Draw a pixel in a framebuffer
 *  @param[in] this The pointer to SRPFramebuffer, as returned 
 *                    from srpNewFramebuffer
 *  @param[in] x,y Position at which to draw the pixel. X-axis points
 *                 from left to right, Y-axis - from top to bottom
 *  @param[in] depth Depth value to assign to the pixel, assumed to be inside
 *                   the [-1, 1] interval. If it is less than currently assigned
 *                   depth buffer value to this pixel, nothing is drawn.
 *  @param[in] color RBGA8888 color to draw. */
void srpFramebufferDrawPixel(
	const SRPFramebuffer* this, size_t x, size_t y, double depth,
	uint32_t color
);

/** Convert Normalized Device Coordinates to screen-space coordiantes
 *  @param[in] this The pointer to SRPFramebuffer, as returned
 *                    from srpNewFramebuffer
 *  @param[in] NDC Pointer to 3-element double array containing NDC position
 *  @param[out] SS Pointer to 3-element double array that will contain SS
 *                   position after the call. The z-component is the same as
 *                   z-component of NDC coordinates. */
void srpFramebufferNDCToScreenSpace
	(const SRPFramebuffer* this, const double NDC[2], double SS[2]);

/** @} */  // ingroup Framebuffer

