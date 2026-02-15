// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Fragment emission implementation */

#include <math.h>
#include "fragment.h"
#include "color.h"
#include "math_utils.h"

void emitFragment(
    const SRPFramebuffer* fb, const SRPShaderProgram* sp,
    int x, int y, SRPfsInput* fsIn
)
{
    SRPfsOutput fsOut = {
        .color = {0},
        .fragDepth = NAN
    };

    sp->fs->shader(fsIn, &fsOut);

    SRPColor color = {
        CLAMP(0, 255, fsOut.color[0] * 255),
        CLAMP(0, 255, fsOut.color[1] * 255),
        CLAMP(0, 255, fsOut.color[2] * 255),
        CLAMP(0, 255, fsOut.color[3] * 255)
    };

    // If depth wasn't overridden by the user's fragment shader
    double depth = isnan(fsOut.fragDepth) ? fsIn->fragCoord[2] : fsOut.fragDepth;

    if (framebufferDepthTest(fb, x, y, depth))
        framebufferDrawPixel(fb, x, y, depth, SRP_COLOR_TO_UINT32_T(color));
}
