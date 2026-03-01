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
	SRP_FLOAT = 0,
	SRP_DOUBLE,
	SRP_INT8,
	SRP_INT16,
	SRP_INT32,
	SRP_INT64,
	SRP_UINT8,
	SRP_UINT16,
	SRP_UINT32,
	SRP_UINT64
} SRPType;

/** @} */  // ingroup Various
