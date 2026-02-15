// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Line rasterization implementation */

#include <math.h>
#include <stdlib.h>
#include "line.h"
#include "fragment.h"
#include "context.h"
#include "utils.h"
#include "message_callback_p.h"

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

    /** @todo what if steps = 0? edge case */
    int steps = (int) ceil(fmax(fabs(dx), fabs(dy)));

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

	if (perspective)
    {
        pPosition->w = 1 / ( line->invW[0] * (1-t) + line->invW[1] * t );
        pPosition->z = pPosition->w * (
            line->v[0].position[2] * line->invW[0] * (1-t) + \
            line->v[1].position[2] * line->invW[1] * t
        );
    }
	else  // affine
    {
        pPosition->w = 1.;
        pPosition->z = line->v[0].position[2] * (1-t) + line->v[1].position[2] * t;
    }
}

static void lineInterpolateAttributes(
	SRPLine* line, double t, const SRPShaderProgram* restrict sp,
	vec4d* pPosition, SRPInterpolated* pInterpolatedBuffer
)
{
	// vertices[i].pOutputVariables =
	// (                        Vi                              )
	// (          ViA0          )(          ViA1          ) ...
	// (ViA0E0 ViA0E1 ... ViA0En)(ViA1E0 ViA1E1 ... ViA1En) ...
	// [V]ertex, [A]ttribute, [E]lement

	bool perspective = (srpContext.interpolationMode == SRP_INTERPOLATION_MODE_PERSPECTIVE);

	// Points to current attribute in output buffer
	void* pInterpolatedAttrVoid = pInterpolatedBuffer;

	size_t attrOffsetBytes = 0;
	for (size_t attrI = 0; attrI < sp->vs->nOutputVariables; attrI++)
	{
		SRPVertexVariableInformation* attr = &sp->vs->outputVariablesInfo[attrI];
		size_t elemSize = 0;
		switch (attr->type)
		{
		case TYPE_DOUBLE:
		{
			elemSize = sizeof(double);
			double* pInterpolatedAttr = (double*) pInterpolatedAttrVoid;

			// Pointers to the current attribute of 0th and 1st vertices
			double* AV[2];
			for (int i = 0; i < 2; i++)
				AV[i] = (double*) ADD_VOID_PTR(line->v[i].pOutputVariables, attrOffsetBytes);

			if (perspective)
				for (size_t elemI = 0; elemI < attr->nItems; elemI++)
				{
					pInterpolatedAttr[elemI] = pPosition->w * (
						AV[0][elemI] * line->invW[0] * (1-t) + \
						AV[1][elemI] * line->invW[1] * t
					);
				}
			else  // affine
				for (size_t elemI = 0; elemI < attr->nItems; elemI++)
				{
					pInterpolatedAttr[elemI] = \
						AV[0][elemI] * (1-t) + \
						AV[1][elemI] * t;
				}
			break;
		}
		default:
			srpMessageCallbackHelper(
				SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
				"Unexpected type (%i)", attr->type
			);
		}

		size_t attrSize = elemSize * attr->nItems;
		pInterpolatedAttrVoid = ADD_VOID_PTR(pInterpolatedAttrVoid, attrSize);
		attrOffsetBytes += attrSize;
	}
}
