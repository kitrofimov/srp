// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Rasterization
 *  Fragment emission implementation */

#include <math.h>
#include <assert.h>
#include "raster/fragment.h"
#include "srp/color.h"
#include "math/utils.h"

/** @ingroup Rasterization
 *  @{ */

void emitFragment(
    const SRPFramebuffer* fb, const SRPShaderProgram* sp,
    int x, int y, SRPfsInput* fsIn
)
{
    assert(x >= 0 && x < fb->width);
    assert(y >= 0 && y < fb->width);

    const bool overwrite = sp->fs->mayOverwriteDepth;
    const float interpolatedDepth = fsIn->fragCoord[2];

    SRPfsOutput fsOut = {
        .color = {0},
        .fragDepth = NAN
    };

    float depth = interpolatedDepth;

    uint32_t* pColor;
    float* pDepth;
    framebufferGetColorAndDepthPointers(fb, x, y, &pColor, &pDepth);
    const float storedDepth = *pDepth;

    // Early depth test
    if (!overwrite && depth <= storedDepth)
        return;

    sp->fs->shader(fsIn, &fsOut);

    if (overwrite)
    {
        // Resolve final depth
        if (!isnan(fsOut.fragDepth))
            depth = fsOut.fragDepth;

        if (depth <= storedDepth)
            return;
    }

    SRPColor color = {
        CLAMP(0, 255, fsOut.color[0] * 255),
        CLAMP(0, 255, fsOut.color[1] * 255),
        CLAMP(0, 255, fsOut.color[2] * 255),
        CLAMP(0, 255, fsOut.color[3] * 255)
    };

	// If this is failed, this is the problem of library code
	// Not a direct check because of floating point imprecisions
	assert(ROUGHLY_GREATER_OR_EQUAL(depth, -1) && ROUGHLY_LESS_OR_EQUAL(depth, 1));

	*pColor = SRP_COLOR_TO_UINT32_T(color);
	*pDepth = depth;
}

/** @} */  // ingroup Rasterization
