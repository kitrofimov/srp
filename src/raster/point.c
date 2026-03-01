// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Rasterization
 *  Point rasterization implementation */

#include <math.h>
#include "raster/point.h"
#include "raster/fragment.h"
#include "pipeline/vertex_processing.h"
#include "srp/context.h"
#include "srp/color.h"
#include "math/utils.h"

/** @ingroup Rasterization
 *  @{ */

/** Given screen-space point position, compute its math and raster boundaries.
 *  @param[in] ss Screen-space point position
 *  @param[in] pointSize Point size, in pixels
 *  @param[in] fb Pointer to the framebuffer, used to clip partially-OOB points
 *  @param[out] outMinBP,outMaxBP Bounding points representing mathematical boundaries 
 *  @param[out] outMinX,outMaxX,outMinY,outMaxY Raster boundaries
 *  @return `true` if point is fully OOB, `false` otherwise
 */
static bool computeMathAndRasterBoundaries(
    vec3 ss, float pointSize, const SRPFramebuffer* fb, vec2* outMinBP, vec2* outMaxBP,
    int* outMinX, int* outMaxX, int* outMinY, int* outMaxY
);

void rasterizePoint(
    SRPPoint* point, const SRPFramebuffer* fb,
    const SRPShaderProgram* restrict sp
)
{
    const float pointSize = srpContext.pointSize;
    vec3 ss;
    vec2 minBP, maxBP;
    int minX, maxX, minY, maxY;

    framebufferNDCToScreenSpace(fb, point->v.ndcPosition, (float*) &ss);
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
            const float px = x + 0.5;
            const float py = y + 0.5;

            // Square coverage test
            if (px < minBP.x || px >= maxBP.x || py < minBP.y || py >= maxBP.y)
                continue;

            // No need to interpolate anything, just redirect outputs from
            // vertex shader to fragment shader
            SRPFragmentShaderIn fsIn = {
                .uniform = sp->uniform,
                .varyings = (SRPInterpolated*) point->v.varyings,
                .fragCoord = {px, py, point->v.ndcPosition[2], point->v.ndcPosition[3]},
                .frontFacing = true,
                .primitiveID = point->id,
            };
            emitFragment(fb, sp, x, y, &fsIn);
        }
    }
}

void setupPoint(SRPPoint* p)
{
    applyPerspectiveDivide(&p->v, NULL);
}

static bool computeMathAndRasterBoundaries(
    vec3 ss, float pointSize, const SRPFramebuffer* fb, vec2* outMinBP, vec2* outMaxBP,
    int* outMinX, int* outMaxX, int* outMinY, int* outMaxY
)
{
    float halfSize = pointSize * 0.5;

    vec2 minBP = {
        ss.x - halfSize,
        ss.y - halfSize,
    };
    vec2 maxBP = {
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

/** @} */  // ingroup Rasterization
