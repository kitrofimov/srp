// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

/** @file
 *  @ingroup Various
 *  Functions and types to help dealing with opaque types */

#include <stdint.h>
#include <stddef.h>

/** @ingroup Various
 *  @{ */

/** Represents data types. This is needed for internal handling of opaque types */
typedef enum SRPType
{
	SRP_UINT8 = 0,
	SRP_UINT16,
	SRP_UINT32,
	SRP_UINT64,
	SRP_FLOAT,
	SRP_DOUBLE
} SRPType;

/** @} */  // ingroup Various
