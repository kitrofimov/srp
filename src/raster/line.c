// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Rasterization
 *  Line rasterization implementation */

#include <math.h>
#include <stdlib.h>
#include "raster/line.h"
#include "raster/fragment.h"
#include "pipeline/interpolation.h"
#include "pipeline/vertex_processing.h"
#include "srp/context.h"
#include "utils/voidptr.h"
#include "utils/message_callback_p.h"

/** @ingroup Rasterization
 *  @{ */

/** Interpolate the fragment position and vertex variables inside the line.
 *  @param[in] line Line to interpolate data for
 *  @param[in] t Interpolation parameter (0 -> 0th vertex; 1 -> 1st vertex)
 *  @param[in] sp A pointer to shader program to use
 *  @param[out] pInterpolatedBuffer A pointer to the buffer where interpolated
 *              variables will appear. Must be big enough to hold all of them
 *  @param[out] depth Fragment depth
 *  @param[out] recIntInvW Reciprocal of interpolated inverse Wclip */
static void lineInterpolateData(
	SRPLine* line, float t, const SRPShaderProgram* restrict sp,
	SRPInterpolated* pInterpolatedBuffer, float* depth, float* recIntInvW
);

void rasterizeLine(
	SRPLine* line, const SRPFramebuffer* fb,
	const SRPShaderProgram* restrict sp, void* interpolatedBuffer
)
{
    const vec3* ss = line->ss;  // alias

    float dx = ss[1].x - ss[0].x;
    float dy = ss[1].y - ss[0].y;

    int steps = (int) ceil(fmax(fabs(dx), fabs(dy)));
	if (steps == 0)
		steps = 1;

    float xInc = dx / steps;
    float yInc = dy / steps;
    float tInc = 1. / steps;

    float x = ss[0].x;
    float y = ss[0].y;
    float t = 0.;

    for (int i = 0; i <= steps; i++)
    {
        int px = (int) round(x);
        int py = (int) round(y);

        float depth, recIntInvW;
        lineInterpolateData(line, t, sp, interpolatedBuffer, &depth, &recIntInvW);

        SRPFragmentShaderIn fsIn = {
            .uniform = sp->uniform,
            .varyings = interpolatedBuffer,
            .fragCoord = { px + 0.5, py + 0.5, depth, recIntInvW },
            .frontFacing = true,
            .primitiveID = line->id,
        };
        emitFragment(fb, sp, px, py, &fsIn);

        x += xInc;
        y += yInc;
        t += tInc;
    }
}

void setupLine(SRPLine* line, const SRPFramebuffer* fb) 
{
	for (uint8_t i = 0; i < 3; i++)
		applyPerspectiveDivide(&line->v[i], &line->invW[i]);

    for (uint8_t i = 0; i < 2; i++)
        framebufferNDCToScreenSpace(fb, line->v[i].ndcPosition, (float*) &line->ss[i]);
}

static void lineInterpolateData(
	SRPLine* line, float t, const SRPShaderProgram* restrict sp,
	SRPInterpolated* pInterpolatedBuffer, float* depth, float* recIntInvW
)
{
	const bool perspective = srpContext.interpolationMode == SRP_INTERPOLATION_MODE_PERSPECTIVE;
	const float weights[2] = {1-t, t};
	interpolateDepthAndWLine(line->v, weights, line->invW, perspective, sp, depth, recIntInvW);
	interpolateAttributes(line->v, 2, weights, line->invW, *recIntInvW, perspective, sp, pInterpolatedBuffer);
}

/** @} */  // ingroup Rasterization
