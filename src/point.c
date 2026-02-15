// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Point rasterization implementation */

#include <math.h>
#include "point.h"
#include "context.h"
#include "color.h"
#include "math_utils.h"

#include <stdio.h>

void rasterizePoint(
    SRPPoint* point, const SRPFramebuffer* fb,
    const SRPShaderProgram* restrict sp
)
{
    vec3d ss;
    srpFramebufferNDCToScreenSpace(fb, point->v.position, (double*) &ss);

    const double pointSize = srpContext.pointSize;
    const double halfSize = pointSize * 0.5;

    vec2d minBP = {
        ss.x - halfSize,
        ss.y - halfSize,
    };
    vec2d maxBP = {
        ss.x + halfSize,
        ss.y + halfSize,
    };

    // Convert to integer pixel bounds
    size_t minX = (size_t) floor(minBP.x);
    size_t maxX = (size_t) floor(maxBP.x);
    size_t minY = (size_t) floor(minBP.y);
    size_t maxY = (size_t) floor(maxBP.y);

    /** @todo should clipping be done earlier? */
    // Clip to framebuffer
    if (minX >= fb->width || minY >= fb->height)
        return;
    if (maxX >= fb->width)  maxX = fb->width - 1;
    if (maxY >= fb->height) maxY = fb->height - 1;

    for (size_t y = minY; y <= maxY; y++)
    {
        for (size_t x = minX; x <= maxX; x++)
        {
            // Pixel center
            const double px = x + 0.5;
            const double py = y + 0.5;

            // Square coverage test
            if (px < minBP.x || px >= maxBP.x || py < minBP.y || py >= maxBP.y)
                continue;

            // No need to interpolate anything, just redirect from vertex shader to fragment shader
            SRPfsInput fsIn = {
                .uniform = sp->uniform,
                .interpolated = (SRPInterpolated*) point->v.pOutputVariables,
                .fragCoord = {x, y, point->v.position[3], point->v.position[4]},
                .frontFacing = true,
                .primitiveID = point->id,
            };
            SRPfsOutput fsOut = {0};

            sp->fs->shader(&fsIn, &fsOut);

            SRPColor color = {
                CLAMP(0, 255, fsOut.color[0] * 255),
                CLAMP(0, 255, fsOut.color[1] * 255),
                CLAMP(0, 255, fsOut.color[2] * 255),
                CLAMP(0, 255, fsOut.color[3] * 255)
            };
            /** @todo `SRPfsOutput.fragDepth` may be set to 0 manually by the user */
            double depth = (fsOut.fragDepth == 0) ? fsIn.fragCoord[2] : fsOut.fragDepth;

            srpFramebufferDrawPixel(fb, x, y, depth, SRP_COLOR_TO_UINT32_T(color));
        }
    }
}
