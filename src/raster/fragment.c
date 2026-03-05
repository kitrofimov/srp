// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Rasterization
 *  Fragment emission implementation */

#include <math.h>
#include <assert.h>
#include "raster/fragment.h"
#include "srp/context.h"
#include "srp/color.h"
#include "math/utils.h"

/** @ingroup Rasterization
 *  @{ */

/** Perform a depth test, respecting the user-set compare operation and
 *  whether or not the test is enabled
 *  @param[in] incoming The depth value of the currently processed fragment
 *  @param[in] stored The depth value stored in the depth buffer
 *  @return `true` if depth test passes, `false` otherwise */
static inline bool depthTest(float incoming, float stored);

/** Perform a scissor test, respecting the user-set runtime options
 *  @param[in] x,y Window-space position of the currently processed fragment
 *  @return `true` if scissor test passes, `false` otherwise */
static inline bool scissorTest(size_t x, size_t y);


void emitFragment(
    const SRPFramebuffer* fb, const SRPShaderProgram* sp,
    int x, int y, SRPFragmentShaderIn* fsIn
)
{
    assert(x >= 0 && x < fb->width);
    assert(y >= 0 && y < fb->height);

    if (!scissorTest(x, y))
        return;

    const bool overwrite = sp->fs->mayOverwriteDepth;
    const float interpolatedDepth = fsIn->fragCoord[2];
    const bool test = srpContext.depth.testEnable;
    const bool write = srpContext.depth.writeEnable;

    SRPFragmentShaderOut fsOut = {
        .color = {0},
        .fragDepth = NAN
    };

    float depth = interpolatedDepth;

    uint32_t* pColor;
    float* pDepth;
    framebufferGetColorAndDepthPointers(fb, x, y, &pColor, &pDepth);
    const float storedDepth = *pDepth;

    // Early depth test
    if (!overwrite && !depthTest(depth, storedDepth))
        return;

    sp->fs->shader(fsIn, &fsOut);

    if (overwrite)
    {
        // Resolve final depth
        if (!isnan(fsOut.fragDepth))
            depth = fsOut.fragDepth;

        if (!depthTest(depth, storedDepth))
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
    if (test && write)  // Cannot write when not testing
        *pDepth = depth;
}

static inline bool scissorTest(size_t x, size_t y)
{
    if (!srpContext.scissor.enabled)  // Always pass when disabled
        return true;

    const size_t x0 = srpContext.scissor.x;
    const size_t x1 = srpContext.scissor.x + srpContext.scissor.width;
    const size_t y0 = srpContext.scissor.y;
    const size_t y1 = srpContext.scissor.y + srpContext.scissor.height;

    if (x < x0 || x >= x1 || y < y0 || y >= y1)
        return false;
    return true;
}

static inline bool depthTest(float incoming, float stored)
{
    const SRPDepthState* depth = &srpContext.depth;

    if (!depth->testEnable)  // Always pass when disabled
        return true;

    switch (depth->compareOp)
    {
        case SRP_COMPARE_NEVER:    return false;
        case SRP_COMPARE_ALWAYS:   return true;
        case SRP_COMPARE_LESS:     return incoming <  stored;
        case SRP_COMPARE_LEQUAL:   return incoming <= stored;
        case SRP_COMPARE_GREATER:  return incoming >  stored;
        case SRP_COMPARE_GEQUAL:   return incoming >= stored;
        case SRP_COMPARE_EQUAL:    return incoming == stored;
        case SRP_COMPARE_NOTEQUAL: return incoming != stored;
    }

    return false;
}

/** @} */  // ingroup Rasterization
