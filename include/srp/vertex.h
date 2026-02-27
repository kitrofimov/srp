// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

/** @file
 *  @ingroup Vertex
 *  Typedefs related to vertex handling */

#include <stddef.h>
#include "type.h"

/** @ingroup Vertex
 *  @{ */

/** User-defined vertex type, stored in SRPVertexBuffer */
typedef struct SRPVertex SRPVertex;

/** Represents a vertex variable as outputted by vertex shader
 *  @see `SRPvsOutput` */
typedef struct SRPVertexVariable SRPVertexVariable;

/** Holds information needed to interpolate vertex variables inside the primitive
 *  @see `SRPVertexVariable` */
typedef struct
{
	size_t nItems;  /**< How many items does this attribute have */
	SRPType type;   /**< The type of each attribute */
} SRPVertexVariableInformation;

/** Represents interpolated vertex shader's output variables
 *  @see SRPVertexVariable */
typedef struct SRPInterpolated SRPInterpolated;

/** @} */  // ingroup Vertex
