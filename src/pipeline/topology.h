// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Topology-related functions */

#pragma once

#include "core/buffer_p.h"

/** Deduce the number of triangles to assemble based on the number of vertices
 *  and primitive type
 *  @param[in] vertexCount Number of vertices
 *  @param[in] prim Primitive type
 *  @return Number of triangles that should be assembled */
size_t computeTriangleCount(size_t vertexCount, SRPPrimitive prim);
 
/** Determine the stream indices for a triangle based on its type
 *  @param[in] base Base stream index
 *  @param[in] rawTriIdx Index of the primitive to assemble, starting from 0,
 * 						 including culled triangles
 *  @param[in] prim Primitive type
 *  @param[out] out Array of size 3 to store the resulting stream indices */
void resolveTriangleTopology(
	size_t base, size_t rawTriIdx, SRPPrimitive prim, size_t* out
);

/** Deduce the number of lines to assemble based on the number of vertices
 *  and primitive type */
size_t computeLineCount(size_t vertexCount, SRPPrimitive prim);

/** Determine the stream indices for a line based on its type
 *  @param[in] base Base stream index
 *  @param[in] rawLineIdx Index of the primitive to assemble, starting from 0,
 * 						  including culled/skipped lines
 *  @param[in] prim Primitive type
 *  @param[in] vertexCount The total amount of stream vertices for this draw call
 *  @param[out] out Array of size 2 to store the resulting stream indices */
void resolveLineTopology(
	size_t base, size_t rawLineIdx, SRPPrimitive prim, size_t vertexCount, size_t* out
);
