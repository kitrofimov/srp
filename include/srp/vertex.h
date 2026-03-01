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

/** User-defined vertex type, stored in SRPVertexBuffer */
typedef struct SRPVertex SRPVertex;

/** Represents a varying as outputted by the vertex shader
 *  @see `SRPVertexShaderOut` */
typedef struct SRPVarying SRPVarying;

/** Holds information needed to interpolate varyings inside the primitive
 *  @see `SRPVarying` */
typedef struct
{
	size_t nItems;  /**< How many items does this attribute have */
	SRPType type;   /**< The type of each attribute */
} SRPVaryingInfo;

/** Represents interpolated varyings
 *  @see SRPVarying */
typedef struct SRPInterpolated SRPInterpolated;

/** @} */  // ingroup Vertex
