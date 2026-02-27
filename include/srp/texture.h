// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

/** @file
 *  @ingroup Texture
 *  SRPTexture and related functions */

#include <stdint.h>
#include <stddef.h>
#include "srp/color.h"

/** @ingroup Texture
 *  @{ */

/** Holds texture wrapping modes @see SRPTexture */
typedef enum
{
	TW_REPEAT,        /**< Repeat the texture when it's wrapped */
	TW_CLAMP_TO_EDGE  /**< Clamp the texture to the edge when it's wrapped */
} SRPTextureWrappingMode;

/** A structure to represent a texture */
typedef struct SRPTexture SRPTexture;

/** Initialize a texture by loading an image
 *  @param[in] image A filesystem path to an image. Most popular image types
 *					 are supported
 *	@param[in] wrappingModeX,wrappingModeY Wrapping modes to use in X and Y axes
 *	@return A pointer to the constructed texture */
SRPTexture* srpNewTexture(
	const char* image,
	SRPTextureWrappingMode wrappingModeX,
	SRPTextureWrappingMode wrappingModeY
);
/** Free a texture
 *  @param[in] this A pointer to the texture, as returned by srpNewTexture() */
void srpFreeTexture(SRPTexture* this);

/** Get a texel color at u, v by filtering the texture using specified modes
 *  @param[in] this A pointer to the texture, as returned by srpNewTexture()
 *  @param[in] u,v Coordinates of the requested texel
 *  @param[out] out An array of 4 values each in [0, 1] interval, representing
 *                  RGBA8888 color (0.0 = 0x0; 1.0 = 0xFF) */
void srpTextureGetFilteredColor(
	const SRPTexture* this, float u, float v, float out[4]
);

/** Holds all possible arguments to SRPTexture getters and setters
 *  @see srpTextureGet() srpTextureSet() */
typedef enum SRPTextureParameter
{
	SRP_TEXTURE_WRAPPING_MODE_X,
	SRP_TEXTURE_WRAPPING_MODE_Y
} SRPTextureParameter;

 /** Get a parameter from existing SRPTexture
  *  @param[in] this A pointer to texture
  *  @param[in] parameter A struct member to get
  *  @return Requested member, or -1 on failure */
int srpTextureGet(SRPTexture* this, SRPTextureParameter parameter);
 /** Set a parameter to existing SRPTexture
  *  @param[in] this A pointer to texture
  *  @param[in] parameter A struct member to set to
  *  @param[in] data A value to set */
void srpTextureSet(SRPTexture* this, SRPTextureParameter parameter, int data);

/** @} */  // ingroup Texture
