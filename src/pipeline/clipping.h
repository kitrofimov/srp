// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

#include "raster/triangle.h"

typedef enum {
    PLANE_LEFT,
    PLANE_RIGHT,
    PLANE_BOTTOM,
    PLANE_TOP,
    PLANE_NEAR,
    PLANE_FAR,
    PLANE_COUNT
} ClipPlane;

/** Clip the triangle using Sutherland-Hodgman algorithm
 *  @param[in] in The triangle to clip
 *  @param[in] sp The shader program being used
 *  @param[out] out The returned array of triangles
 *  @return Amount of outputted triangles */
size_t clipTriangle(const SRPTriangle* in, const SRPShaderProgram* sp, SRPTriangle* out);
