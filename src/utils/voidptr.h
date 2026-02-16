// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

/** @file
 *  Void pointer utility macros */

#include <stdint.h>

/** @ingroup Various_internal
 *  @{ */

/** Index `void*` as if it was an array with `nBytesPerElement` bytes per element */
#define INDEX_VOID_PTR(ptr, idx, nBytesPerElement) \
	( ( (uint8_t*) (ptr) ) + (idx) * (nBytesPerElement) )
/** Add `nBytes` bytes to the `void*` */
#define ADD_VOID_PTR(ptr, nBytes) \
	( ( (uint8_t*) (ptr) ) + (nBytes) )

/** @} */  // ingroup Various_internal

