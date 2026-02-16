// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "srp/framebuffer.h"
#include "utils/message_callback_p.h"
#include "utils/defines.h"

/** @file
 *  Framebuffer implementation */

/** @ingroup Framebuffer_internal
 *  @{ */

/** Get pointer to a pixel inside color buffer
 *  @param[in] this Pointer to SRPFramebuffer
 *  @param[in] x,y Position of requested pixel
 *  @return Requested pointer */
static uint32_t* framebufferGetPixelPointer
	(const SRPFramebuffer* this, size_t x, size_t y);

/** Get pointer to a pixel inside depth buffer
 *  @param[in] this Pointer to SRPFramebuffer
 *  @param[in] x,y Position of requested pixel
 *  @return Requested pointer */
static double* framebufferGetDepthPointer
	(const SRPFramebuffer* this, size_t x, size_t y);

/** @} */  // ingroup Framebuffer_internal

SRPFramebuffer* srpNewFramebuffer(size_t width, size_t height)
{
	SRPFramebuffer* this = SRP_MALLOC(sizeof(SRPFramebuffer));

	this->width = width;
	this->height = height;
	this->size = width * height;
	this->color = SRP_MALLOC(sizeof(uint32_t) * this->size);
	this->depth = SRP_MALLOC(sizeof(double) * this->size);

	return this;
}

void srpFreeFramebuffer(SRPFramebuffer* this)
{
	SRP_FREE(this->color);
	SRP_FREE(this->depth);
	SRP_FREE(this);
}

// The `const` qualifier is completely legal: only the buffer pointed to by
// `framebuffer->color` is modified, not the structure itself!
void framebufferDrawPixel(
	const SRPFramebuffer* this, size_t x, size_t y, double depth,
	uint32_t color
)
{
	// If this is failed, this is the problem of library code =>
	// => no point to use message callback
	assert(depth >= -1 && depth < 1);
	*framebufferGetPixelPointer(this, x, y) = color;
	*framebufferGetDepthPointer(this, x, y) = depth;
}

bool framebufferDepthTest(
	const SRPFramebuffer* this, size_t x, size_t y, double depth
)
{
	double* pStoredDepth = framebufferGetDepthPointer(this, x, y);
	return (depth > *pStoredDepth);
}

void framebufferNDCToScreenSpace(
	const SRPFramebuffer* this, const double* NDC, double* SS
)
{
	SS[0] =  (((double) this->width  - 1) / 2) * (NDC[0] + 1);
	SS[1] = -(((double) this->height - 1) / 2) * (NDC[1] - 1),
	SS[2] = NDC[2];
}

void srpFramebufferClear(const SRPFramebuffer* this)
{
	memset(this->color, 0x00, this->size * sizeof(uint32_t));
    for (size_t i = 0; i < this->size; i++)
        this->depth[i] = -1.;
}

static uint32_t* framebufferGetPixelPointer(const SRPFramebuffer* this, size_t x, size_t y)
{
	return this->color + (y * this->width + x);
}

static double* framebufferGetDepthPointer(const SRPFramebuffer* this, size_t x, size_t y)
{
	return this->depth + (y * this->width + x);
}

