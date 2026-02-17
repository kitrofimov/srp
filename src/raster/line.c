// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Line rasterization implementation */

#include <math.h>
#include <stdlib.h>
#include "raster/line.h"
#include "raster/fragment.h"
#include "pipeline/interpolation.h"
#include "srp/context.h"
#include "utils/voidptr.h"
#include "utils/message_callback_p.h"

/** Interpolate the fragment position and vertex variables inside the line.
 *  @param[in] line Line to interpolate data for
 *  @param[in] t Interpolation parameter (0 -> 0th vertex; 1 -> 1st vertex)
 *  @param[in] sp A pointer to shader program to use
 *  @param[out] pPosition A pointer to vec4d where interpolated position will appear.
 *  @param[out] pInterpolatedBuffer A pointer to the buffer where interpolated
 *              variables will appear. Must be big enough to hold all of them */
static void lineInterpolateData(
	SRPLine* line, double t, const SRPShaderProgram* restrict sp,
	vec4d* pPosition, SRPInterpolated* pInterpolatedBuffer
);

/** Interpolate the fragment position inside the line.
 *  @param[in] line Line to interpolate position for
 *  @param[in] t Interpolation parameter (0 -> 0th vertex; 1 -> 1st vertex)
 *  @param[out] pPosition A pointer to vec4d where interpolated position will appear. */
static void lineInterpolatePosition(SRPLine* line, double t, vec4d* pPosition);

/** Interpolate vertex attributes inside the line.
 *  @param[in] line Line to interpolate attributes for
 *  @param[in] t Interpolation parameter (0 -> 0th vertex; 1 -> 1st vertex)
 *  @param[in] sp A pointer to shader program to use
 *  @param[in] pPosition A pointer to the interpolated position
 *  @param[out] pInterpolatedBuffer A pointer to the buffer where interpolated
 *              variables will appear. Must be big enough to hold all of them */
static void lineInterpolateAttributes(
	SRPLine* line, double t, const SRPShaderProgram* restrict sp,
	vec4d* pPosition, SRPInterpolated* pInterpolatedBuffer
);

void rasterizeLine(
	SRPLine* line, const SRPFramebuffer* fb,
	const SRPShaderProgram* restrict sp, void* interpolatedBuffer
)
{
    const vec3d* ss = line->ss;  // alias

    double dx = ss[1].x - ss[0].x;
    double dy = ss[1].y - ss[0].y;

    int steps = (int) ceil(fmax(fabs(dx), fabs(dy)));
	if (steps == 0)
		steps = 1;

    double xInc = dx / steps;
    double yInc = dy / steps;
    double tInc = 1. / steps;

    double x = ss[0].x;
    double y = ss[0].y;
    double t = 0.;

    for (int i = 0; i <= steps; i++)
    {
        int px = (int) round(x);
        int py = (int) round(y);

        vec4d interpolatedPosition = {0};
        lineInterpolateData(line, t, sp, &interpolatedPosition, interpolatedBuffer);

        SRPfsInput fsIn = {
            .uniform = sp->uniform,
            .interpolated = interpolatedBuffer,
            .fragCoord = {
                px + 0.5,
                py + 0.5,
                interpolatedPosition.z,
                interpolatedPosition.w
            },
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
    for (uint8_t i = 0; i < 2; i++)
        framebufferNDCToScreenSpace(fb, line->v[i].position, (double*) &line->ss[i]);
}

static void lineInterpolateData(
	SRPLine* line, double t, const SRPShaderProgram* restrict sp,
	vec4d* pPosition, SRPInterpolated* pInterpolatedBuffer
)
{
    lineInterpolatePosition(line, t, pPosition);
    lineInterpolateAttributes(line, t, sp, pPosition, pInterpolatedBuffer);
}

static void lineInterpolatePosition(SRPLine* line, double t, vec4d* pPosition)
{
	bool perspective = (srpContext.interpolationMode == SRP_INTERPOLATION_MODE_PERSPECTIVE);

	pPosition->x = line->v[0].position[0] * (1-t) + line->v[1].position[0] * t;
	pPosition->y = line->v[0].position[1] * (1-t) + line->v[1].position[1] * t;

	// If I am not mistaken, this is linear in screen space too
	pPosition->z = line->v[0].position[2] * (1-t) + line->v[1].position[2] * t;

	if (perspective)
        pPosition->w = 1 / ( line->invW[0] * (1-t) + line->invW[1] * t );
	else  // affine
        pPosition->w = 1.;
}

static void lineInterpolateAttributes(
	SRPLine* line, double t, const SRPShaderProgram* restrict sp,
	vec4d* pPosition, SRPInterpolated* pInterpolatedBuffer
)
{
	const bool perspective = srpContext.interpolationMode == SRP_INTERPOLATION_MODE_PERSPECTIVE;
	const double weights[2] = {1-t, t};
	interpolateAttributes(line->v, 2, weights, line->invW, pPosition->w, perspective, sp, pInterpolatedBuffer);
}
