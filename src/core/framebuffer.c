// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Framebuffer_internal
 *  Framebuffer implementation */

#include <stdio.h>
#include <string.h>
#include "core/framebuffer_p.h"
#include "utils/message_callback_p.h"
#include "math/utils.h"

/** @ingroup Framebuffer_internal
 *  @{ */

SRPFramebuffer* srpNewFramebuffer(size_t width, size_t height)
{
	SRPFramebuffer* this = SRP_MALLOC(sizeof(SRPFramebuffer));
	this->width = width;
	this->height = height;
	this->size = width * height;
	this->color = SRP_MALLOC(sizeof(uint32_t) * this->size);
	this->depth = SRP_MALLOC(sizeof(float) * this->size);
	this->stencil = SRP_MALLOC(sizeof(uint8_t) * this->size);
	return this;
}

void srpFreeFramebuffer(SRPFramebuffer* this)
{
	SRP_FREE(this->color);
	SRP_FREE(this->depth);
	SRP_FREE(this->stencil);
	SRP_FREE(this);
}

SRP_FORCEINLINE void framebufferGetPointers(
	const SRPFramebuffer* this, size_t x, size_t y,
	uint32_t** pColor, float** pDepth, uint8_t** pStencil
)
{
    size_t idx = y * this->width + x;
    *pColor = &this->color[idx];
    *pDepth = &this->depth[idx];
	*pStencil = &this->stencil[idx];
}

void framebufferNDCToScreenSpace(
    const SRPFramebuffer* this, const float* NDC, float* SS
)
{
    SS[0] = ((float) this->width  / 2.) * (NDC[0] + 1.);
    SS[1] = ((float) this->height / 2.) * (1. - NDC[1]);
    SS[2] = NDC[2];
}

void srpFramebufferClear(const SRPFramebuffer* this)
{
	memset(this->color, 0x00, this->size * sizeof(uint32_t));
    for (size_t i = 0; i < this->size; i++)
        this->depth[i] = -1.;
}

/** @} */  // ingroup Framebuffer_internal
