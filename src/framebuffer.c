// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "framebuffer.h"
#include "message_callback_p.h"
#include "defines.h"

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
void srpFramebufferDrawPixel(
	const SRPFramebuffer* this, size_t x, size_t y, double depth,
	uint32_t color
)
{
	/** @todo This is not the job of the framebuffer to check this,
	 *  this should be deleted after the primitive clipping implementation */
	if (1 < depth || depth < -1)
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Depth value is not inside [-1, 1] interval; depth=%lf", depth
		);

	double* pDepth = framebufferGetDepthPointer(this, x, y);
	if (depth < *pDepth)
		return;

	*framebufferGetPixelPointer(this, x, y) = color;
	*pDepth = depth;
}

void srpFramebufferNDCToScreenSpace(
	const SRPFramebuffer* this, const double NDC[2], double SS[2]
)
{
	SS[0] =  (((double) this->width  - 1) / 2) * (NDC[0] + 1);
	SS[1] = -(((double) this->height - 1) / 2) * (NDC[1] - 1);
}

void framebufferClear(const SRPFramebuffer* this)
{
	for (size_t y = 0; y < this->height; y++)
	{
		for (size_t x = 0; x < this->width; x++)
		{
			*framebufferGetPixelPointer(this, x, y) = 0x000000FF;
			*framebufferGetDepthPointer(this, x, y) = -1;
		}
	}
}

uint32_t* framebufferGetPixelPointer(const SRPFramebuffer* this, size_t x, size_t y)
{
	return this->color + (y * this->width + x);
}

double* framebufferGetDepthPointer(const SRPFramebuffer* this, size_t x, size_t y)
{
	return this->depth + (y * this->width + x);
}

