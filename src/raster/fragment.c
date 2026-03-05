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
#include "utils/message_callback_p.h"

/** @ingroup Rasterization
 *  @{ */

/** Perform a scissor test, respecting the user-set runtime options
 *  @param[in] x,y Window-space position of the currently processed fragment
 *  @return `true` if scissor test passes, `false` otherwise */
static inline bool scissorTest(size_t x, size_t y);

/** Perform an initial stage of the stencil test and potentially apply fail
 *  operation to the stencil value. Checks if the stencil test is enabled.
 *  @param[in] stencil Pointer to the current stencil value */
static inline bool stencilTest(uint8_t* stencil);

/** Compare two masked stencil values with a certain operation
 *  @param[in] ref The reference stencil value
 *  @param[in] stored The stored stencil value
 *  @param[in] mask The mask to use with both values
 *  @param[in] op Operation to compare with */
static inline bool stencilCompare(uint8_t ref, uint8_t stored, uint8_t mask, SRPCompareOp op);

/** Apply a "stencil test failed" operation to a stencil value
 *  @param[in] stencil Pointer to the current stencil value */
static inline void stencilFailOp(uint8_t* stencil);

/** Apply a "stencil test passed, depth test failed" operation to a stencil value
 *  @param[in] stencil Pointer to the current stencil value */
static inline void stencilDepthFailOp(uint8_t* stencil);

/** Apply a "stencil and depth tests passed" operation to a stencil value
 *  @param[in] stencil Pointer to the current stencil value */
static inline void stencilPassOp(uint8_t* stencil);

/** Apply an operation to a stencil value */
static inline uint8_t applyStencilOp(SRPStencilOp op, uint8_t stored, uint8_t ref);

/** Perform a depth test, respecting the user-set compare operation and
 *  whether or not the test is enabled
 *  @param[in] incoming The depth value of the currently processed fragment
 *  @param[in] stored The depth value stored in the depth buffer
 *  @return `true` if depth test passes, `false` otherwise */
static inline bool depthTest(float incoming, float stored);


void emitFragment(
    const SRPFramebuffer* fb, const SRPShaderProgram* sp,
    int x, int y, SRPFragmentShaderIn* fsIn
)
{
    assert(x >= 0 && x < fb->width);
    assert(y >= 0 && y < fb->height);

    if (!scissorTest(x, y))
        return;

    uint32_t* pColor; float* pDepth; uint8_t* pStencil;
    framebufferGetPointers(fb, x, y, &pColor, &pDepth, &pStencil);
    
    const bool earlyDepthTest = !sp->fs->mayOverwriteDepth;
    const bool stencilTestEnabled = srpContext.stencil.enabled;
    const float storedDepth = *pDepth;
    float depth = fsIn->fragCoord[2];

    if (!stencilTest(pStencil))
        return;

    bool depthPassed = true;
    if (earlyDepthTest)
    {
        depthPassed = depthTest(depth, storedDepth);
        if (!depthPassed)
        {
            if (stencilTestEnabled)
                stencilDepthFailOp(pStencil);
            return;
        }
    }

    SRPFragmentShaderOut fsOut = { .color = {0}, .fragDepth = NAN };
    sp->fs->shader(fsIn, &fsOut);

    if (!earlyDepthTest)
    {
        if (!isnan(fsOut.fragDepth))
            depth = fsOut.fragDepth;
        
        depthPassed = depthTest(depth, storedDepth);
        if (!depthPassed)
        {
            if (stencilTestEnabled)
                stencilDepthFailOp(pStencil);
            return;
        }
    }

    if (stencilTestEnabled)
        stencilPassOp(pStencil);

    // If this is failed, this is the problem of library code
    // Not a direct check because of floating point imprecisions
    assert(ROUGHLY_GREATER_OR_EQUAL(depth, -1) && ROUGHLY_LESS_OR_EQUAL(depth, 1));

    SRPColor color = {
        CLAMP(0, 255, fsOut.color[0] * 255),
        CLAMP(0, 255, fsOut.color[1] * 255),
        CLAMP(0, 255, fsOut.color[2] * 255),
        CLAMP(0, 255, fsOut.color[3] * 255)
    };
    *pColor = SRP_COLOR_TO_UINT32_T(color);

    // Cannot write when not testing (mimicking OpenGL behaviour)
    if (srpContext.depth.testEnable && srpContext.depth.writeEnable)
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

static inline bool stencilTest(uint8_t* stencil)
{
    if (!srpContext.stencil.enabled)
        return true;

    const SRPStencilState* s = &srpContext.stencil;
    bool passed = stencilCompare(s->ref, *stencil, s->mask, s->func);
    if (!passed)
    {
        stencilFailOp(stencil);
        return false;
    }
    return true;
}

static inline bool stencilCompare(uint8_t ref, uint8_t stored, uint8_t mask, SRPCompareOp op)
{
    uint8_t lhs = ref & mask;
    uint8_t rhs = stored & mask;

    switch (op)
    {
        case SRP_COMPARE_NEVER:    return false;
        case SRP_COMPARE_ALWAYS:   return true;
        case SRP_COMPARE_LESS:     return lhs <  rhs;
        case SRP_COMPARE_LEQUAL:   return lhs <= rhs;
        case SRP_COMPARE_GREATER:  return lhs >  rhs;
        case SRP_COMPARE_GEQUAL:   return lhs >= rhs;
        case SRP_COMPARE_EQUAL:    return lhs == rhs;
        case SRP_COMPARE_NOTEQUAL: return lhs != rhs;
        default:
            srpMessageCallbackHelper(
                SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
                "Unknown stencil operation (%i), fallback to SRP_COMPARE_NEVER", op
            );
            return false;
    }
}

static inline void stencilFailOp(uint8_t* stencil)
{
    uint8_t val = applyStencilOp(srpContext.stencil.sfailOp, *stencil, srpContext.stencil.ref);
    *stencil = val & srpContext.stencil.writeMask;
}

static inline void stencilDepthFailOp(uint8_t* stencil)
{
    uint8_t val = applyStencilOp(srpContext.stencil.dfailOp, *stencil, srpContext.stencil.ref);
    *stencil = val & srpContext.stencil.writeMask;
}

static inline void stencilPassOp(uint8_t* stencil)
{
    uint8_t val = applyStencilOp(srpContext.stencil.passOp, *stencil, srpContext.stencil.ref);
    *stencil = val & srpContext.stencil.writeMask;
}

static inline uint8_t applyStencilOp(SRPStencilOp op, uint8_t stored, uint8_t ref)
{
    switch (op)
    {
        case SRP_STENCIL_KEEP:      return stored;
        case SRP_STENCIL_ZERO:      return 0;
        case SRP_STENCIL_REPLACE:   return ref;
        case SRP_STENCIL_INCR:      return (stored < 255) ? stored + 1 : 255;
        case SRP_STENCIL_INCR_WRAP: return stored + 1;
        case SRP_STENCIL_DECR:      return (stored > 0) ? stored - 1 : 0;
        case SRP_STENCIL_DECR_WRAP: return stored - 1;
        case SRP_STENCIL_INVERT:    return ~stored;
        default:
            srpMessageCallbackHelper(
                SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
                "Unknown stencil operation (%i), fallback to SRP_STENCIL_KEEP", op
            );
            return stored;
    }
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
