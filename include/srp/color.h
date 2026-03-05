// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Framebuffer
 *  SRPColor */

#pragma once

#include <stdint.h>

/** @ingroup Framebuffer
 *  @{ */

/** Holds RGBA8888 color data */
typedef struct SRPColor
{
	uint8_t r, g, b, a;
} SRPColor;

/** @} */  // ingroup Framebuffer
