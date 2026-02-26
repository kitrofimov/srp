// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include "srp/shaders.h"
#include "srp/vec.h"

void interpolateDepthAndWTriangle(
    SRPvsOutput* vertices, const float* weights, const float* invW,
    bool perspective, const SRPShaderProgram* sp,
    float* depth, float* reciprocalInterpolatedInvW
);

void interpolateDepthAndWLine(
    SRPvsOutput* vertices, const float* weights, const float* invW,
    bool perspective, const SRPShaderProgram* sp,
    float* depth, float* reciprocalInterpolatedInvW
);

void interpolateAttributes(
    SRPvsOutput* vertices, size_t nVertices, const float* weights,
    const float* invW, float reciprocalInterpolatedInvW, bool perspective,
    const SRPShaderProgram* sp, SRPInterpolated* pOutput
);
