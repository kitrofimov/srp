// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Framebuffer_internal
 *  SRPColor internal */

#include "core/color_p.h"
#include "math/utils.h"

/** @ingroup Framebuffer
 *  @{ */

inline uint32_t colorPack(float color[4])
{
    SRPColor c = {
        CLAMP(0, 255, color[0] * 255),
        CLAMP(0, 255, color[1] * 255),
        CLAMP(0, 255, color[2] * 255),
        CLAMP(0, 255, color[3] * 255)
    };
    return SRP_COLOR_TO_UINT32_T(c);
}

/** @} */  // ingroup Framebuffer_internal
