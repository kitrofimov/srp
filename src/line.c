// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Line rasterization implementation */

#include <math.h>
#include <stdlib.h>
#include "line.h"
#include "fragment.h"
#include "vec.h"

void rasterizeLine(
	SRPLine* line, const SRPFramebuffer* fb,
	const SRPShaderProgram* restrict sp
)
{
    vec3d ss[2];
    for (uint8_t i = 0; i < 2; i++)
        framebufferNDCToScreenSpace(fb, line->v[i].position, (double*) &ss[i]);

    double dx = ss[1].x - ss[0].x;
    double dy = ss[1].y - ss[0].y;

    int steps = (int) ceil(fmax(abs(dx), abs(dy)));

    double xInc = dx / steps;
    double yInc = dy / steps;

    double x = ss[0].x;
    double y = ss[0].y;

    for (int i = 0; i <= steps; i++)
    {
        int px = (int) round(x);
        int py = (int) round(y);

        SRPfsInput fsIn = {
            .uniform = sp->uniform,
            .interpolated = NULL,  // TODO
            .fragCoord = {
                px + 0.5,
                py + 0.5,
                0,  // TODO
                1
            },
            .frontFacing = true,
            .primitiveID = line->id,
        };
        emitFragment(fb, sp, px, py, &fsIn);

        x += xInc;
        y += yInc;
    }
}
