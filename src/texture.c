// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include "texture.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "message_callback_p.h"
#include "defines.h"
#include "math_utils.h"
#include "stb_image.h"
#include "utils.h"
#include "vec.h"
#include "texture_p.h"

/** @file
 *  Texture implementation */

/** @ingroup Texture_internal
 *  @{ */

/** Number of channels requested from the `stbi_load()` call. */
#define N_CHANNELS_REQUESTED 3

/** @} */  // ingroup Texture_internal

SRPTexture* srpNewTexture(
	const char* image,
	SRPTextureWrappingMode wrappingModeX, SRPTextureWrappingMode wrappingModeY,
	SRPTextureFilteringMode filteringModeMagnifying,
	SRPTextureFilteringMode filteringModeMinifying
)
{
	SRPTexture* this = SRP_MALLOC(sizeof(SRPTexture));
	this->data = stbi_load(image, &this->width, &this->height, NULL, N_CHANNELS_REQUESTED);
	if (this->data == NULL)
	{
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Failed to load image `%s`: %s", image, stbi_failure_reason()
		);
		return NULL;
	}
	this->wdthMinusOne = this->width - 1;
	this->heightMinusOne = this->height - 1;
	this->wrappingModeX = wrappingModeX;
	this->wrappingModeY = wrappingModeY;
	this->filteringModeMagnifying = filteringModeMagnifying;
	this->filteringModeMinifying = filteringModeMinifying;
	return this;
}

void srpFreeTexture(SRPTexture* this)
{
	stbi_image_free(this->data);
	SRP_FREE(this);
}

/** @todo Now using only filteringModeMagnifying; how to know if texture is 
 *  magnified or minified?
 *  @todo Too many conditionals?
 *  @todo wrappingMode is untested! */
void srpTextureGetFilteredColor(
	const SRPTexture* this, double u, double v, double out[4]
)
{
	if (u < 0 || u > 1)
	{
		switch (this->wrappingModeX)
		{
		case TW_REPEAT:
			u = FRACTIONAL(u);
			break;
		case TW_CLAMP_TO_EDGE:
			u = CLAMP(0., 1., u);
			break;
		default:
			srpMessageCallbackHelper(
				SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
				"Unknown texture wrapping mode (%i)", this->wrappingModeX
			);
			u = 0.;
		}
	}
	if (v < 0 || v > 1)
	{
		switch (this->wrappingModeY)
		{
		case TW_REPEAT:
			v = FRACTIONAL(v);
			break;
		case TW_CLAMP_TO_EDGE:
			v = CLAMP(0., 1., v);
			break;
		default:
			srpMessageCallbackHelper(
				SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
				"Unknown texture wrapping mode (%i)", this->wrappingModeY
			);
			v = 0.;
		}
	}

	// V axis is pointed down-up, but images are stored up-down, so (1-v) here
	double x = this->wdthMinusOne * u;
	double y = this->heightMinusOne * (1-v);
	size_t xi = (size_t) (x + 0.5);
	size_t yi = (size_t) (y + 0.5);

	switch (this->filteringModeMagnifying)
	{
	case TF_NEAREST:
	{
        // Each pixel is N_CHANNELS_REQUESTED bytes, and an image is stored row-major
        uint8_t* start = \
            INDEX_VOID_PTR(this->data, xi + yi * this->width, N_CHANNELS_REQUESTED);
        const double inv255 = 1. / 255.;
        out[0] = start[0] * inv255;
        out[1] = start[1] * inv255;
        out[2] = start[2] * inv255;
        out[3] = (N_CHANNELS_REQUESTED == 3) ? 1. : (start[3] * inv255);
		return;
	}
	}
}

int srpTextureGet(SRPTexture* this, SRPTextureParameter parameter)
{
	switch (parameter)
	{
	case SRP_TEXTURE_WRAPPING_MODE_X:
		return this->wrappingModeX;
	case SRP_TEXTURE_WRAPPING_MODE_Y:
		return this->wrappingModeY;
	case SRP_TEXTURE_FILTERING_MODE_MAGNIFYING:
		return this->filteringModeMagnifying;
	case SRP_TEXTURE_FILTERING_MODE_MINIFYING:
		return this->filteringModeMinifying;
	default:
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Unknown texture parameter (%i)", parameter
		);
		return -1;
	}
}

void srpTextureSet(SRPTexture* this, SRPTextureParameter parameter, int data)
{
	switch (parameter)
	{
	case SRP_TEXTURE_WRAPPING_MODE_X:
		this->wrappingModeX = data;
		return;
	case SRP_TEXTURE_WRAPPING_MODE_Y:
		this->wrappingModeY = data;
		return;
	case SRP_TEXTURE_FILTERING_MODE_MAGNIFYING:
		this->filteringModeMagnifying = data;
		return;
	case SRP_TEXTURE_FILTERING_MODE_MINIFYING:
		this->filteringModeMinifying = data;
		return;
	default:
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Unknown texture parameter (%i)", parameter
		);
		return;
	}
}

