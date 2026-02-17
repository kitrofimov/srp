// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <string.h>
#include <assert.h>
#include "pipeline/clipping.h"
#include "memory/arena_p.h"
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
static inline bool insidePlane(const SRPvsOutput* v, ClipPlane p);
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

        if (polyCount == 0)  // Fully clipped
            return 0;

        for (size_t i = 0; i < polyCount; i++)  // Swap buffers
            poly[i] = temp[i];
    }

    // Triangulate (fan)
    /** @todo is the winding right here? */
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

        bool currInside = insidePlane(current, plane);
        bool nextInside = insidePlane(next, plane);

        if (currInside && nextInside)
        {
            deepCopyVertex(next, varyingSize, &out[outCount]);
            outCount++;
        }
        else if (currInside != nextInside)  // only one is outside
        {
            double da = planeDistance(current, plane);
            double db = planeDistance(next, plane);
            double t = da / (da - db);

            interpolateVertex(current, next, t, sp, &out[outCount]);
            outCount++;

            if (!currInside && nextInside)
            {
                deepCopyVertex(next, varyingSize, &out[outCount]);
                outCount++;
            }
        }
        // else both outside -> emit nothing
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

    /** @todo duplicate interpolation logic here, in line.c and in triangle.c */

	// Points to current attribute in output buffer
	void* pInterpolatedAttrVoid = ARENA_ALLOC(sp->vs->nBytesPerOutputVariables);
    out->pOutputVariables = pInterpolatedAttrVoid;

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
			double* AV[2] = {
                (double*) ADD_VOID_PTR(a->pOutputVariables, attrOffsetBytes),
                (double*) ADD_VOID_PTR(b->pOutputVariables, attrOffsetBytes)
            };

            for (size_t elemI = 0; elemI < attr->nItems; elemI++)
                pInterpolatedAttr[elemI] = AV[0][elemI] * (1-t) + AV[1][elemI] * t;

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

static void deepCopyVertex(const SRPvsOutput* src, size_t varyingSize, SRPvsOutput* dst)
{
    *dst = *src;
    void* mem = ARENA_ALLOC(varyingSize);
    memcpy(mem, src->pOutputVariables, varyingSize);
    dst->pOutputVariables = mem;
}

static inline bool insidePlane(const SRPvsOutput* v, ClipPlane p)
{
    const double x = v->position[0];
    const double y = v->position[1];
    const double z = v->position[2];
    const double w = v->position[3];

    switch (p)
    {
        case PLANE_LEFT:   return x >= -w;
        case PLANE_RIGHT:  return x <=  w;
        case PLANE_BOTTOM: return y >= -w;
        case PLANE_TOP:    return y <=  w;
        case PLANE_NEAR:   return z >= -w;
        case PLANE_FAR:    return z <=  w;
        default:           assert(false);
    }
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
