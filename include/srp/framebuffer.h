// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  SRPFramebuffer and related functions */

#pragma once

#include <stddef.h>
#include <stdint.h>

/** @ingroup Framebuffer
 *  @{ */

/** Holds RBGA8888 color buffer and depth buffer */
typedef struct SRPFramebuffer
{
	size_t width;     /**< Width */
	size_t height;    /**< Height */
	size_t size;      /**< N of pixels (width*height) */
	uint32_t* color;  /**< Pointer to color buffer */
	double* depth;    /**< Pointer to depth buffer */
} SRPFramebuffer;

/** Create a framebuffer
 *  @param[in] width Width of a new framebuffer in pixels
 *  @param[in] height Height of a new framebuffer in pixels
 *  @return A pointer to the created framebuffer */
SRPFramebuffer* srpNewFramebuffer(size_t width, size_t height);

/** Free a framebuffer
 *  @param[in] this The pointer to SRPFramebuffer, as returned from srpNewFramebuffer() */
void srpFreeFramebuffer(SRPFramebuffer* this);

/** Clear a framebuffer: fill the color with black and depth with -1
 *  @param[in] this The pointer to SRPFramebuffer, as returned from srpNewFramebuffer() */
void srpFramebufferClear(const SRPFramebuffer* this);

/** @} */  // ingroup Framebuffer

