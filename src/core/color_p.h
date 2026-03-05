// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Framebuffer_internal
 *  SRPColor internal header */

#pragma once

#include "srp/color.h"
#ifdef _WIN32
	#include <winsock.h>
#else
	#include <arpa/inet.h>
#endif

/** @ingroup Framebuffer
 *  @{ */

/** Convert the `SRPColor` to `uint32_t` RGBA8888 */
// Using `htonl` since we need to convert host endian data to big endian data
// (we want red component to be the most significant one!)
#define SRP_COLOR_TO_UINT32_T(color) (htonl(*(uint32_t*) &color))

/** Pack a color represented via `float[4]` into `uint32_t` RGBA8888 */
uint32_t colorPack(float color[4]);

/** @} */  // ingroup Framebuffer_internal
