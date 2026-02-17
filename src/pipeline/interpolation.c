// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include "pipeline/interpolation.h"
#include "utils/message_callback_p.h"
#include "utils/voidptr.h"

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

void interpolatePosition(
    SRPvsOutput* vertices, size_t nVertices, const double* weights,
    const double* invW, bool perspective, const SRPShaderProgram* sp,
    vec4d* pPosition
)
{
    for (size_t i = 0; i < 3; i++)
    {
        double sum = 0.;
        for (size_t j = 0; j < nVertices; j++)
            sum += vertices[j].position[i] * weights[j];
        ((double*) pPosition)[i] = sum;
    }

	if (perspective)
    {
        double sum = 0.;
        for (size_t j = 0; j < nVertices; j++)
            sum += invW[j] * weights[j];
        pPosition->w = 1 / sum;
    }
	else  // affine
        pPosition->w = 1.;
}

void interpolateAttributes(
    SRPvsOutput* vertices, size_t nVertices, const double* weights,
    const double* invW, double reciprocalInterpolatedInvW, bool perspective,
    const SRPShaderProgram* sp, SRPInterpolated* pOutput
)
{
	// vertices[i].pOutputVariables =
	// (                        Vi                              )
	// (          ViA0          )(          ViA1          ) ...
	// (ViA0E0 ViA0E1 ... ViA0En)(ViA1E0 ViA1E1 ... ViA1En) ...
	// [V]ertex, [A]ttribute, [E]lement

    void* pAttrVoid = pOutput;
    size_t attrOffsetBytes = 0;

    for (size_t attrI = 0; attrI < sp->vs->nOutputVariables; attrI++)
    {
        SRPVertexVariableInformation* attr = &sp->vs->outputVariablesInfo[attrI];
        size_t elemSize = 0;

        // Pointers to the current attribute of each vertex
        void* AV[nVertices];

        switch (attr->type)
        {
        case TYPE_DOUBLE:
            elemSize = sizeof(double);
            double* pInterpolatedAttr = (double*) pAttrVoid;

			for (int i = 0; i < 3; i++)
				AV[i] = ADD_VOID_PTR(vertices[i].pOutputVariables, attrOffsetBytes);

            for (size_t elemI = 0; elemI < attr->nItems; elemI++)
            {
                double sum = 0.;

                if (perspective)
                {
                    for (size_t i = 0; i < nVertices; i++)
                        sum += ((double*) AV[i])[elemI] * invW[i] * weights[i];
                    sum *= reciprocalInterpolatedInvW;
                }
                else
                    for (size_t i = 0; i < nVertices; i++)
                        sum += ((double*) AV[i])[elemI] * weights[i];

                pInterpolatedAttr[elemI] = sum;
            }
            break;

        default:
            srpMessageCallbackHelper(
                SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
                "Unexpected type (%i)", attr->type
            );
        }

        size_t attrSize = elemSize * attr->nItems;
        pAttrVoid = ADD_VOID_PTR(pAttrVoid, attrSize);
        attrOffsetBytes += attrSize;
    }
}
