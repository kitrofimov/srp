// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  SRPColor */

#pragma once

#include <stdint.h>
#ifdef _WIN32
	#include <winsock.h>
#else
	#include <arpa/inet.h>
#endif

/** @ingroup Framebuffer
 *  @{ */

/** Holds RGBA8888 color data */
typedef struct SRPColor
{
	uint8_t r, g, b, a;
} SRPColor;

/** Convert SRPColor to `uint32_t` RGBA8888 */
// Using `htonl` since we need to convert host endian data to big endian data
// (we want red component to be the most significant one!)
#define SRP_COLOR_TO_UINT32_T(color) (htonl(*(uint32_t*) &color))

/** @} */  // ingroup Framebuffer

