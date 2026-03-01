// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Interpolation
 *  Interpolation implementation */

#include <string.h>
#include "pipeline/interpolation.h"
#include "utils/message_callback_p.h"
#include "utils/voidptr.h"

/** @ingroup Interpolation
 *  @{ */

/** Let @f$ v_0, v_1, v_2 @f$ be points in space that form a triangle and
 *  @f$ a, b, c @f$ be barycentric coordinates for a point @f$ P @f$
 *  according to @f$ v_0, v_1, v_2 @f$ respectively. Then, by the property
 *  of barycentric coordinates @f$ P = av_0 + bv_1 + cv_2 @f$. This can be
 *  extrapolated to arbitrary values assigned to vertices, and this is called
 *  affine attribute interpolation.
 *
 *  But this method does not take the perspective divide into account, so
 *  the texture (for example) will look "wrong".
 *
 *  It can be shown (see the "see also" section) that perspective-correct Z
 *  value can be obtained by dividing affinely-interpolated Ai/Wi by
 *  affinely-interpolated 1/Wi.
 *
 *  @see https://www.comp.nus.edu.sg/%7Elowkl/publications/lowk_persp_interp_techrep.pdf
 *  @see https://www.youtube.com/watch?v=F5X6S35SW2s */

void interpolateDepthAndWTriangle(
    SRPVertexShaderOut* vertices, const float* weights, const float* invW,
    const SRPShaderProgram* sp, float* depth, float* reciprocalInterpolatedInvW
)
{
    *reciprocalInterpolatedInvW = 1 / (
        invW[0] * weights[0] + \
        invW[1] * weights[1] + \
        invW[2] * weights[2]
    );
    *depth = vertices[0].ndcPosition[2] * invW[0] * weights[0] + \
             vertices[1].ndcPosition[2] * invW[1] * weights[1] + \
             vertices[2].ndcPosition[2] * invW[2] * weights[2];
}

void interpolateDepthAndWLine(
    SRPVertexShaderOut* vertices, const float* weights, const float* invW,
    const SRPShaderProgram* sp, float* depth, float* reciprocalInterpolatedInvW
)
{
    *reciprocalInterpolatedInvW = 1 / (
        invW[0] * weights[0] + \
        invW[1] * weights[1]
    );
    *depth = vertices[0].ndcPosition[2] * invW[0] * weights[0] + \
             vertices[1].ndcPosition[2] * invW[1] * weights[1];
}

/** Interpolate a floating point type attribute @see interpolateAttribute */
#define INTERPOLATE_FLOATING(Type) \
do { \
    elemSize = sizeof(Type); \
    Type* interpolated = (Type*) interpolatedVoid; \
    for (size_t elemI = 0; elemI < attr->nItems; elemI++) \
    { \
        Type value = 0.; \
        if (perspective) \
        { \
            for (size_t i = 0; i < nVertices; i++) \
                value += ((Type*) AV[i])[elemI] * invW[i] * weights[i]; \
            value *= reciprocalInterpolatedInvW; \
        } \
        else if (affine) \
            for (size_t i = 0; i < nVertices; i++) \
                value += ((Type*) AV[i])[elemI] * weights[i]; \
        else  /* flat */ \
            value = ((Type*) AV[provokingVertex])[elemI]; \
        interpolated[elemI] = value; \
    } \
} while(0)

/** Flat-interpolate an integer type attribute @see interpolateAttribute */
#define INTERPOLATE_INTEGER(Type) \
do { \
    elemSize = sizeof(Type); \
    Type* interpolated = (Type*) interpolatedVoid; \
    memcpy(interpolated, AV[provokingVertex], elemSize * attr->nItems); \
} while(0)

void interpolateAttributes(
    SRPVertexShaderOut* vertices, size_t nVertices, const float* weights,
    const float* invW, float reciprocalInterpolatedInvW, 
    const SRPShaderProgram* sp, SRPInterpolated* pOutput
)
{
	// vertices[i].varyings =
	// (                        Vi                              )
	// (          ViA0          )(          ViA1          ) ...
	// (ViA0E0 ViA0E1 ... ViA0En)(ViA1E0 ViA1E1 ... ViA1En) ...
	// [V]ertex, [A]ttribute, [E]lement

    void* interpolatedVoid = pOutput;
    size_t attrOffsetBytes = 0;
    const size_t provokingVertex = 0;  // TODO

    for (size_t attrI = 0; attrI < sp->vs->nVaryings; attrI++)
    {
        SRPVaryingInfo* attr = &sp->vs->varyingsInfo[attrI];
        bool perspective = attr->interpolationMode == SRP_INTERPOLATION_MODE_PERSPECTIVE;
        bool affine      = attr->interpolationMode == SRP_INTERPOLATION_MODE_AFFINE;
        
        if (invW == NULL)  // see clipping.c interpolateVertex
        {
            perspective = false;
            affine = true;
        }

        size_t elemSize = 0;

        void* AV[nVertices];  // Pointers to the current attribute of each vertex
        for (size_t i = 0; i < nVertices; i++)
            AV[i] = ADD_VOID_PTR(vertices[i].varyings, attrOffsetBytes);

        switch (attr->type)
        {
        case SRP_FLOAT:
            INTERPOLATE_FLOATING(float); break;
        case SRP_DOUBLE:
            INTERPOLATE_FLOATING(double); break;

        case SRP_INT8:
            INTERPOLATE_INTEGER(int8_t); break;
        case SRP_INT16:
            INTERPOLATE_INTEGER(int16_t); break;
        case SRP_INT32:
            INTERPOLATE_INTEGER(int32_t); break;
        case SRP_INT64:
            INTERPOLATE_INTEGER(int64_t); break;
        case SRP_UINT8:
            INTERPOLATE_INTEGER(uint8_t); break;
        case SRP_UINT16:
            INTERPOLATE_INTEGER(uint16_t); break;
        case SRP_UINT32:
            INTERPOLATE_INTEGER(uint32_t); break;
        case SRP_UINT64:
            INTERPOLATE_INTEGER(uint64_t); break;

        default:
            srpMessageCallbackHelper(
                SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
                "Unexpected type (%i)", attr->type
            );
        }

        size_t attrSize = elemSize * attr->nItems;
        interpolatedVoid = ADD_VOID_PTR(interpolatedVoid, attrSize);
        attrOffsetBytes += attrSize;
    }
}

/** @} */  // ingroup Interpolation
