// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Clipping
 *  Clipping implementation */

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "pipeline/clipping.h"
#include "pipeline/interpolation.h"
#include "memory/arena_p.h"
#include "math/utils.h"
#include "utils/message_callback_p.h"
#include "utils/voidptr.h"

/** @ingroup Clipping
 *  @{ */

/** Represents all possible clip planes */
typedef enum {
    PLANE_LEFT,
    PLANE_RIGHT,
    PLANE_BOTTOM,
    PLANE_TOP,
    PLANE_NEAR,
    PLANE_FAR,
    PLANE_COUNT
} ClipPlane;

/** Compute the clip code for a vertex. 0 if inside all 6 clip planes,
 *  else 1 in a specific bit.
 *  @param[in] v Vertex
 *  @return Clip code */
static inline uint8_t computeClipCode(const SRPVertexShaderOut* v);

/** Clip a polygon against specified plane (Sutherland-Hodgman)
 *  @param[in] in Vertices of an input polygon
 *  @param[in] inCount Amount of vertices the input polygon has
 *  @param[in] plane The plane to clip against
 *  @param[in] sp The shader program being used
 *  @param[out] out Vertices of a clipped polygon
 *  @return Amount of vertices the clipped polygon has */
static size_t clipAgainstPlane(
    SRPVertexShaderOut* in, size_t inCount, ClipPlane plane,
    const SRPShaderProgram* sp, SRPVertexShaderOut* out
);

/** Create new vertex via interpolating between two existing ones
 *  @param[in] a First vertex
 *  @param[in] b Second vertex
 *  @param[in] t Interpolation parameter: 0 -> first vertex, 1 -> second vertex
 *  @param[in] sp The shader program being used
 *  @param[in] out Interpolated vertex */ 
static void interpolateVertex(
    const SRPVertexShaderOut* a, const SRPVertexShaderOut* b, float t,
    const SRPShaderProgram* sp, SRPVertexShaderOut* out
);

/** Calculate the distance from the vertex to the specified clip plane
 *  @param[in] v Vertex
 *  @param[in] p Clip plane
 *  @return The distance from the vertex to the specified clip plane */
static inline float planeDistance(const SRPVertexShaderOut* v, ClipPlane p);


size_t clipTriangle(const SRPTriangle* in, const SRPShaderProgram* sp, SRPTriangle* out)
{
    uint8_t c0 = computeClipCode(&in->v[0]);
    uint8_t c1 = computeClipCode(&in->v[1]);
    uint8_t c2 = computeClipCode(&in->v[2]);

    if ((c0 | c1 | c2) == 0)  // Trivial accept
    {
        out[0] = *in; 
        return 1;
    }

    if ((c0 & c1 & c2) != 0)  // Trivial reject
        return 0;

    SRPVertexShaderOut bufferA[6];
    SRPVertexShaderOut bufferB[6];
    
    SRPVertexShaderOut* src = bufferA;
    SRPVertexShaderOut* dst = bufferB;
    size_t polyCount = 3;

    // Initialize the buffer from input triangle
    for (int i = 0; i < 3; i++)
        src[i] = in->v[i];

    for (int p = 0; p < PLANE_COUNT; p++)
    {
        polyCount = clipAgainstPlane(src, polyCount, (ClipPlane) p, sp, dst);
        assert(polyCount <= 6);

        if (polyCount == 0)  // Fully clipped
            return 0;

        SRPVertexShaderOut* tmp = src;
        src = dst;
        dst = tmp;
    }

    // Triangulate (fan)
    size_t id = 0;
    for (size_t i = 1; i < polyCount-1; i++)
    {
        SRPTriangle* tri = &out[id];
        tri->v[0] = src[0];
        tri->v[1] = src[i];
        tri->v[2] = src[i+1];
        id++;
    }

    return id;
}

static inline uint8_t computeClipCode(const SRPVertexShaderOut* v) {
    uint8_t code = 0;
    const float x = v->clipPosition[0];
    const float y = v->clipPosition[1];
    const float z = v->clipPosition[2];
    const float w = v->clipPosition[3];

    // 0 = inside; 1 = outside
    code |= (x + w < 0) << 0;  // PLANE_LEFT
    code |= (w - x < 0) << 1;  // PLANE_RIGHT
    code |= (y + w < 0) << 2;  // PLANE_BOTTOM
    code |= (w - y < 0) << 3;  // PLANE_TOP
    code |= (z + w < 0) << 4;  // PLANE_NEAR
    code |= (w - z < 0) << 5;  // PLANE_FAR

    return code;
}

bool clipLine(SRPLine* line, const SRPShaderProgram* sp)
{
    uint8_t c0 = computeClipCode(&line->v[0]);
    uint8_t c1 = computeClipCode(&line->v[1]);

    if ((c0 | c1) == 0)  // Trivial accept
        return false;

    if ((c0 & c1) != 0)  // Trivial reject
        return true;

    float t0 = 0., t1 = 1.;

    for (int p = 0; p < PLANE_COUNT; p++)
    {
        float da = planeDistance(&line->v[0], (ClipPlane) p);
        float db = planeDistance(&line->v[1], (ClipPlane) p);

        if (da < 0. && db < 0.)  // Both outside
            return true;

        if (da < 0. || db < 0.)  // Only one outside
        {
            if (ROUGHLY_ZERO(da - db))
                continue;
            float t = da / (da - db);
            if (da < 0.)
                t0 = MAX(t0, t);  // Entry: push t0 forward
            else
                t1 = MIN(t1, t);  // Exit: pull t1 back

            if (t0 > t1)
                return true;  // Clipped to nothing
        }
    }

    SRPVertexShaderOut A = line->v[0];
    SRPVertexShaderOut B = line->v[1];
    if (t0 > 0.)
        interpolateVertex(&A, &B, t0, sp, &line->v[0]);
    if (t1 < 1.)
        interpolateVertex(&A, &B, t1, sp, &line->v[1]);

    return false;
}

bool clipPoint(SRPPoint* p)
{
    float x = p->v.clipPosition[0];
    float y = p->v.clipPosition[1];
    float z = p->v.clipPosition[2];
    float w = p->v.clipPosition[3];

    if (x < -w || x > w) return true;
    if (y < -w || y > w) return true;
    if (z < -w || z > w) return true;

    return false;
}

static size_t clipAgainstPlane(
    SRPVertexShaderOut* in, size_t inCount, ClipPlane plane,
    const SRPShaderProgram* sp, SRPVertexShaderOut* out
)
{
    if (inCount == 0)
        return 0;

    size_t outCount = 0;

    for (size_t i = 0; i < inCount; i++)
    {
        SRPVertexShaderOut* current = &in[i];
        SRPVertexShaderOut* next = &in[(i + 1) % inCount];

        float da = planeDistance(current, plane);
        float db = planeDistance(next, plane);
        bool currInside = da >= 0;
        bool nextInside = db >= 0;

        if (currInside && nextInside)
        {
            out[outCount] = *next;
            outCount++;
        }
        else if (currInside || nextInside)  // Only one outside
        {
            if (ROUGHLY_ZERO(da - db))
                continue;
            float t = da / (da - db);
            interpolateVertex(current, next, t, sp, &out[outCount]);
            outCount++;

            if (!currInside && nextInside)
            {
                out[outCount] = *next;
                outCount++;
            }
        }
        // Both outside -> emit nothing
    }

    return outCount;
}

static void interpolateVertex(
    const SRPVertexShaderOut* a, const SRPVertexShaderOut* b, float t,
    const SRPShaderProgram* sp, SRPVertexShaderOut* out
)
{
    for (int i = 0; i < 4; i++)
        out->clipPosition[i] = a->clipPosition[i] * (1-t) + b->clipPosition[i] * t;

    void* pVarying = ARENA_ALLOC(sp->vs->varyingsSize);
    out->varyings = pVarying;

    SRPVertexShaderOut vertices[2] = {*a, *b};  /** @todo this is disgusting */
	const float weights[2] = {1-t, t};
    interpolateAttributes(vertices, 2, weights, NULL, 0., false, sp, pVarying);
}

static inline float planeDistance(const SRPVertexShaderOut* v, ClipPlane p)
{
    const float x = v->clipPosition[0];
    const float y = v->clipPosition[1];
    const float z = v->clipPosition[2];
    const float w = v->clipPosition[3];

    switch (p)
    {
        case PLANE_LEFT:   return x + w;
        case PLANE_RIGHT:  return w - x;
        case PLANE_BOTTOM: return y + w;
        case PLANE_TOP:    return w - y;
        case PLANE_NEAR:   return z + w;
        case PLANE_FAR:    return w - z;
        default:           abort();
    }
}

/** @} */  // ingroup Clipping
