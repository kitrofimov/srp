// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <string.h>
#include <assert.h>
#include "pipeline/clipping.h"
#include "pipeline/interpolation.h"
#include "memory/arena_p.h"
#include "math/utils.h"
#include "utils/message_callback_p.h"
#include "utils/voidptr.h"

static size_t clipAgainstPlane(
    SRPvsOutput* in, size_t inCount, ClipPlane plane,
    const SRPShaderProgram* sp, SRPvsOutput* out
);

static void interpolateVertex(
    const SRPvsOutput* a, const SRPvsOutput* b, double t,
    const SRPShaderProgram* sp, SRPvsOutput* out
);

static void deepCopyVertex(const SRPvsOutput* v, size_t varyingSize, SRPvsOutput* out);
static inline double planeDistance(const SRPvsOutput* v, ClipPlane p);

size_t clipTriangle(const SRPTriangle* in, const SRPShaderProgram* sp, SRPTriangle* out)
{
    SRPvsOutput poly[6];
    SRPvsOutput temp[6];
    size_t polyCount = 3;

    const size_t varyingSize = sp->vs->nBytesPerOutputVariables;

    // Initialize working buffers from input triangle
    for (int i = 0; i < 3; i++)
        deepCopyVertex(&in->v[i], varyingSize, &poly[i]);

    // Clip against all 6 planes
    for (int p = 0; p < PLANE_COUNT; p++)
    {
        polyCount = clipAgainstPlane(poly, polyCount, (ClipPlane) p, sp, temp);
        assert(polyCount <= 6);

        if (polyCount == 0)  // Fully clipped
            return 0;

        for (size_t i = 0; i < polyCount; i++)  // Swap buffers
            poly[i] = temp[i];
    }

    // Triangulate (fan)
    size_t id = 0;
    for (size_t i = 1; i < polyCount-1; i++)
    {
        SRPTriangle* tri = &out[id];
        tri->v[0] = poly[0];
        tri->v[1] = poly[i];
        tri->v[2] = poly[i+1];
        id++;
    }

    return id;
}

bool clipLine(SRPLine* line, const SRPShaderProgram* sp)
{
    double t0 = 0., t1 = 1.;

    for (int p = 0; p < PLANE_COUNT; p++)
    {
        double da = planeDistance(&line->v[0], (ClipPlane) p);
        double db = planeDistance(&line->v[1], (ClipPlane) p);

        if (da < 0. && db < 0.)  // Both outside
            return true;

        if (da < 0. || db < 0.)  // Only one outside
        {
            if (ROUGHLY_ZERO(da - db))
                continue;
            double t = da / (da - db);
            if (da < 0.)
                t0 = MAX(t0, t);  // Entry: push t0 forward
            else
                t1 = MIN(t1, t);  // Exit: pull t1 back

            if (t0 > t1)
                return true;  // Clipped to nothing
        }
    }

    SRPvsOutput A = line->v[0];
    SRPvsOutput B = line->v[1];
    if (t0 > 0.)
        interpolateVertex(&A, &B, t0, sp, &line->v[0]);
    if (t1 < 1.)
        interpolateVertex(&A, &B, t1, sp, &line->v[1]);

    return false;
}

static size_t clipAgainstPlane(
    SRPvsOutput* in, size_t inCount, ClipPlane plane,
    const SRPShaderProgram* sp, SRPvsOutput* out
)
{
    if (inCount == 0)
        return 0;

    const size_t varyingSize = sp->vs->nBytesPerOutputVariables;
    size_t outCount = 0;

    for (size_t i = 0; i < inCount; i++)
    {
        SRPvsOutput* current = &in[i];
        SRPvsOutput* next = &in[(i + 1) % inCount];

        double da = planeDistance(current, plane);
        double db = planeDistance(next, plane);
        bool currInside = da > 0;
        bool nextInside = db > 0;

        if (currInside && nextInside)
        {
            deepCopyVertex(next, varyingSize, &out[outCount]);
            outCount++;
        }
        else if (currInside || nextInside)  // Only one outside
        {
            if (ROUGHLY_ZERO(da - db))
                continue;
            double t = da / (da - db);
            interpolateVertex(current, next, t, sp, &out[outCount]);
            outCount++;

            if (!currInside && nextInside)
            {
                deepCopyVertex(next, varyingSize, &out[outCount]);
                outCount++;
            }
        }
        // Both outside -> emit nothing
    }

    return outCount;
}

static void interpolateVertex(
    const SRPvsOutput* a, const SRPvsOutput* b, double t,
    const SRPShaderProgram* sp, SRPvsOutput* out
)
{
    for (int i = 0; i < 4; i++)
        out->position[i] = a->position[i] * (1-t) + b->position[i] * t;

    void* pVarying = ARENA_ALLOC(sp->vs->nBytesPerOutputVariables);
    out->pOutputVariables = pVarying;

    SRPvsOutput vertices[2] = {*a, *b};  /** @todo this is disgusting */
	const double weights[2] = {1-t, t};
    interpolateAttributes(vertices, 2, weights, NULL, 0., false, sp, pVarying);
}

static void deepCopyVertex(const SRPvsOutput* src, size_t varyingSize, SRPvsOutput* dst)
{
    *dst = *src;
    void* mem = ARENA_ALLOC(varyingSize);
    memcpy(mem, src->pOutputVariables, varyingSize);
    dst->pOutputVariables = mem;
}

static inline double planeDistance(const SRPvsOutput* v, ClipPlane p)
{
    const double x = v->position[0];
    const double y = v->position[1];
    const double z = v->position[2];
    const double w = v->position[3];

    switch (p)
    {
        case PLANE_LEFT:   return x + w;
        case PLANE_RIGHT:  return w - x;
        case PLANE_BOTTOM: return y + w;
        case PLANE_TOP:    return w - y;
        case PLANE_NEAR:   return z + w;
        case PLANE_FAR:    return w - z;
        default:           assert(false);
    }
}
