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

/** Get texture color at specified pixel position
 *  @param[in] this Pointer to SRPTexture
 *  @param[in] x,y Position of the requested pixel
 *  @return Requested pixel's color */
static vec4d textureGetColor(const SRPTexture* this, size_t x, size_t y);

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
		}
	}

	// V axis is pointed down-up, but images are stored up-down, so (1-v) here
	double x = (this->width-1) * u;
	double y = (this->height-1) * (1-v);

	switch (this->filteringModeMagnifying)
	{
	case TF_NEAREST:
	{
		vec4d ret = textureGetColor(this, (size_t) (x + 0.5), (size_t) (y + 0.5));
		out[0] = ret.x;
		out[1] = ret.y;
		out[2] = ret.z;
		out[3] = ret.w;
		return;
	}
	}
}

static vec4d textureGetColor(const SRPTexture* this, size_t x, size_t y)
{
	// Each pixel is N_CHANNELS_REQUESTED bytes, and an image is stored row-major
	uint8_t* start = \
		INDEX_VOID_PTR(this->data, x + y * this->width, N_CHANNELS_REQUESTED);
	const double inv255 = 1. / 255.;
	return (vec4d) {
		start[0] * inv255,
		start[1] * inv255,
		start[2] * inv255,
		(N_CHANNELS_REQUESTED == 3) ? 1. : (start[3] * inv255)
	};
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

