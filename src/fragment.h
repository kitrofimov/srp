// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Fragment emission */

#include "framebuffer_p.h"
#include "shaders.h"

/** Run fragment shader, perform depth check and draw a pixel
 *  @param[in] fb The framebuffer to use
 *  @param[in] sp The shader program to use
 *  @param[in] x The x coordinate of the pixel
 *  @param[in] y The y coordinate of the pixel
 *  @param[in] fsIn Fragment shader input
*/
void emitFragment(
    const SRPFramebuffer* fb, const SRPShaderProgram* sp,
    int x, int y, SRPfsInput* fsIn
);
