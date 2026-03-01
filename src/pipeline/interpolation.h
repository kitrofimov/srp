// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Interpolation
 *  Interpolation-related functions */

#pragma once

#include "srp/shaders.h"
#include "srp/vec.h"

/** @ingroup Interpolation
 *  @{ */

/** Interpolate the depth and inverse W values inside the triangle
 *  @param[in] vertices Array of vertices
 *  @param[in] weights Array of barycentric coordinates
 *  @param[in] invW Array of inverseW values for each corresponding vertex
 *  @param[in] perspective Whether or not to perform perspective-correct interpolation,
 *                         else perform affine screen-space interpolation
 *  @param[in] sp The SRPShaderProgram being used
 *  @param[out] depth Where interpolated depth will be stored
 *  @param[out] reciprocalInterpolatedInvW Where the reciprocal of interpolated
 *                                         inverse W_clip will be stored */
void interpolateDepthAndWTriangle(
    SRPVertexShaderOut* vertices, const float* weights, const float* invW,
    bool perspective, const SRPShaderProgram* sp,
    float* depth, float* reciprocalInterpolatedInvW
);

/** Interpolate the depth and inverse W values inside the line
 *  @param[in] vertices Array of vertices
 *  @param[in] weights Array of barycentric coordinates
 *  @param[in] invW Array of inverseW values for each corresponding vertex
 *  @param[in] perspective Whether or not to perform perspective-correct interpolation,
 *                         else perform affine screen-space interpolation
 *  @param[in] sp The SRPShaderProgram being used
 *  @param[out] depth Where interpolated depth will be stored
 *  @param[out] reciprocalInterpolatedInvW Where the reciprocal of interpolated
 *                                         inverse W_clip will be stored */
void interpolateDepthAndWLine(
    SRPVertexShaderOut* vertices, const float* weights, const float* invW,
    bool perspective, const SRPShaderProgram* sp,
    float* depth, float* reciprocalInterpolatedInvW
);

/** Interpolate the attributes inside the primitive
 *  @param[in] vertices Array of vertices
 *  @param[in] nVertices Amount of passed vertices 
 *  @param[in] weights Array of barycentric coordinates
 *  @param[in] invW Array of inverseW values for each corresponding vertex
 *  @param[in] reciprocalInterpolatedInvW The reciprocal of interpolated inverse W_clip
 *  @param[in] perspective Whether or not to perform perspective-correct interpolation,
 *                         else perform affine screen-space interpolation
 *  @param[in] sp The SRPShaderProgram being used
 *  @param[out] pOutput Interpolated vertex attributes */
void interpolateAttributes(
    SRPVertexShaderOut* vertices, size_t nVertices, const float* weights,
    const float* invW, float reciprocalInterpolatedInvW, bool perspective,
    const SRPShaderProgram* sp, SRPInterpolated* pOutput
);

/** @} */  // ingroup Interpolation
