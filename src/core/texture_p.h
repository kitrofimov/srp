// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Texture_internal
 *  Private header for `include/srp/texture.h` */

#pragma once

#include "srp/texture.h"

/** @ingroup Texture_internal
 *  @{ */

struct SRPTexture
{
	uint8_t* data;       /**< The pixel data of the image */
	int width;           /**< X-dimension of the texture */
	int height;          /**< Y-dimension of the texture */
	int widthMinusOne;   /**< Precomputed for optimization */
	int heightMinusOne;  /**< Precomputed for optimization */
	SRPTextureWrappingMode wrappingModeX;  /**< Wrapping mode for X axis */
	SRPTextureWrappingMode wrappingModeY;  /**< Wrapping mode for Y axis */
};

/** @} */  // ingroup Texture_internal
