// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include "srp/shaders.h"

void interpolateAttributes(
    SRPvsOutput* vertices, size_t nVertices, const double* weights,
    const double* invW, double reciprocalInterpolatedW, bool perspective,
    const SRPShaderProgram* sp, SRPInterpolated* pOutput
);
