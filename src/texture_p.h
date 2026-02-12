// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Private header for `include/texture.h` */

#pragma once

#include "texture.h"

/** @ingroup Texture_internal
 *  @{ */

struct SRPTexture
{
	uint8_t* data;
	int width, height;
	int wdthMinusOne, heightMinusOne;
	SRPTextureWrappingMode wrappingModeX;
	SRPTextureWrappingMode wrappingModeY;

	// Not used yet
	SRPTextureFilteringMode filteringModeMagnifying;
	SRPTextureFilteringMode filteringModeMinifying;
};

/** @} */  // ingroup Texture_internal

