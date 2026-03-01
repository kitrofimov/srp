// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Vertex
 *  Typedefs related to vertex handling */

#pragma once

#include <stddef.h>
#include "type.h"

/** @ingroup Vertex
 *  @{ */

/** User-defined vertex type, as stored in SRPVertexBuffer */
typedef struct SRPVertex SRPVertex;

/** Represents a varying as outputted by the vertex shader
 *  @see `SRPVertexShaderOut` */
typedef struct SRPVarying SRPVarying;

/** Varyings interpolation mode */
typedef enum SRPInterpolationMode
{
	/** Perspective-correct interplolation */
	SRP_INTERPOLATION_MODE_PERSPECTIVE,
	/** Affine screen-space interpolation */
	SRP_INTERPOLATION_MODE_AFFINE,
	/** No interpolation, value for the primitive is taken from the provoking vertex */
	SRP_INTERPOLATION_MODE_FLAT
} SRPInterpolationMode;

/** Holds information needed to interpolate varyings inside the primitive
 *  @see `SRPVarying` */
typedef struct
{
	size_t nItems;  /**< How many items does this varying have */
	SRPType type;   /**< The type of the varying */
	/** How the varying should be interpolated inside the primitive */
	SRPInterpolationMode interpolationMode;
} SRPVaryingInfo;

/** Represents interpolated varyings
 *  @see SRPVarying */
typedef struct SRPInterpolated SRPInterpolated;

/** @} */  // ingroup Vertex
