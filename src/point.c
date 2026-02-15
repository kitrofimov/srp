// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Point rasterization implementation */

#include <math.h>
#include "point.h"
#include "context.h"
#include "color.h"
#include "math_utils.h"
#include "fragment.h"

/** Given screen-space point position, compute its math and raster boundaries.
 *  @param[in] ss Screen-space point position
 *  @param[in] pointSize Point size, in pixels
 *  @param[in] fb Pointer to the framebuffer, used to clip partially-OOB points
 *  @return `true` if point is fully OOB, `false` otherwise
 */
static bool computeMathAndRasterBoundaries(
    vec3d ss, double halfSize, const SRPFramebuffer* fb, vec2d* outMinBP, vec2d* outMaxBP,
    int* outMinX, int* outMaxX, int* outMinY, int* outMaxY
);

void rasterizePoint(
    SRPPoint* point, const SRPFramebuffer* fb,
    const SRPShaderProgram* restrict sp
)
{
    const double pointSize = srpContext.pointSize;
    vec3d ss;
    vec2d minBP, maxBP;
    int minX, maxX, minY, maxY;

    framebufferNDCToScreenSpace(fb, point->v.position, (double*) &ss);
    bool success = computeMathAndRasterBoundaries(
        ss, pointSize, fb, &minBP, &maxBP, &minX, &maxX, &minY, &maxY
    );

    if (!success)
        return;

    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
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
                .fragCoord = {px, py, point->v.position[2], point->v.position[3]},
                .frontFacing = true,
                .primitiveID = point->id,
            };
            emitFragment(fb, sp, x, y, &fsIn);
        }
    }
}

static bool computeMathAndRasterBoundaries(
    vec3d ss, double pointSize, const SRPFramebuffer* fb, vec2d* outMinBP, vec2d* outMaxBP,
    int* outMinX, int* outMaxX, int* outMinY, int* outMaxY
)
{
    double halfSize = pointSize * 0.5;

    vec2d minBP = {
        ss.x - halfSize,
        ss.y - halfSize,
    };
    vec2d maxBP = {
        ss.x + halfSize,
        ss.y + halfSize,
    };

    // Convert to integer pixel bounds
    int minX = (int) floor(minBP.x);
    int maxX = (int) floor(maxBP.x);
    int minY = (int) floor(minBP.y);
    int maxY = (int) floor(maxBP.y);

    if (maxX < 0 || maxY < 0 || minX >= (int) fb->width || minY >= (int) fb->height)
        return false;

    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX >= (int) fb->width)  maxX = fb->width - 1;
    if (maxY >= (int) fb->height) maxY = fb->height - 1;

    *outMinBP = minBP;
    *outMaxBP = maxBP;
    *outMinX = minX;
    *outMaxX = maxX;
    *outMinY = minY;
    *outMaxY = maxY;
    return true;
}
