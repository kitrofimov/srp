// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Fragment emission implementation */

#include <math.h>
#include "raster/fragment.h"
#include "srp/color.h"
#include "math/utils.h"

void emitFragment(
    const SRPFramebuffer* fb, const SRPShaderProgram* sp,
    int x, int y, SRPfsInput* fsIn
)
{
    const bool overwrite = sp->fs->doesOverwriteDepth;
    const double interpolatedDepth = fsIn->fragCoord[2];

    SRPfsOutput fsOut = {
        .color = {0},
        .fragDepth = NAN
    };

    double depth = interpolatedDepth;

    // Early depth test
    if (!overwrite && !framebufferDepthTest(fb, x, y, depth))
        return;

    sp->fs->shader(fsIn, &fsOut);

    if (overwrite)
    {
        // Resolve final depth
        if (!isnan(fsOut.fragDepth))
            depth = fsOut.fragDepth;

        if (!framebufferDepthTest(fb, x, y, depth))
            return;
    }

    SRPColor color = {
        CLAMP(0, 255, fsOut.color[0] * 255),
        CLAMP(0, 255, fsOut.color[1] * 255),
        CLAMP(0, 255, fsOut.color[2] * 255),
        CLAMP(0, 255, fsOut.color[3] * 255)
    };

    framebufferDrawPixel(fb, x, y, depth, SRP_COLOR_TO_UINT32_T(color));
}
