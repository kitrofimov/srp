// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include "srp/shaders.h"
#include "srp/vec.h"

void interpolateDepthAndWTriangle(
    SRPvsOutput* vertices, const double* weights, const double* invW,
    bool perspective, const SRPShaderProgram* sp,
    double* depth, double* reciprocalInterpolatedInvW
);

void interpolateDepthAndWLine(
    SRPvsOutput* vertices, const double* weights, const double* invW,
    bool perspective, const SRPShaderProgram* sp,
    double* depth, double* reciprocalInterpolatedInvW
);

void interpolateAttributes(
    SRPvsOutput* vertices, size_t nVertices, const double* weights,
    const double* invW, double reciprocalInterpolatedInvW, bool perspective,
    const SRPShaderProgram* sp, SRPInterpolated* pOutput
);
