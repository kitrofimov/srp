// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Rasterization
 *  Fragment emission */

#pragma once

#include "core/framebuffer_p.h"
#include "srp/shaders.h"

/** @ingroup Rasterization
 *  @{ */

/** Perform (early) depth check, run fragment shader, and draw a pixel
 *  @param[in] fb The framebuffer to use
 *  @param[in] sp The shader program to use
 *  @param[in] x,y Coordinates of the pixel
 *  @param[in] fsIn Fragment shader input
*/
void emitFragment(
    const SRPFramebuffer* fb, const SRPShaderProgram* sp,
    int x, int y, SRPFragmentShaderIn* fsIn
);

/** @} */  // ingroup Rasterization
