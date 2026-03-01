// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

/** @file
 *  @ingroup Shaders
 *  Typedefs related to shaders */

#include <stdbool.h>
#include "srp/vertex.h"

/** @ingroup Shaders
 *  @{ */

/** Represents user-defined shader uniform. Can be accessed by both vertex and
 *  fragment shader via pointer.
 *  @see SRPVertexShaderIn.uniform SRPFragmentShaderIn.uniform SRPShaderProgram */
typedef struct SRPUniform SRPUniform;


/** Holds inputs to vertex shader
 *  @see SRPVertexShader */
typedef struct SRPVertexShaderIn
{
	SRPUniform* uniform;  /**< Pointer to the currently used shader uniform */
	SRPVertex* vertex;    /**< Pointer to the current vertex */
	size_t vertexID;      /**< ID of a current vertex */
} SRPVertexShaderIn;

/** Holds outputs from vertex shader
 *  @see SRPVertexShader */
typedef struct SRPVertexShaderOut
{
	union {
		float clipPosition[4];  /**< Raw vertex shader output */
		float ndcPosition[4];   /**< Position after perspective divide */
	};
	SRPVarying* varyings;   /**< Pointer to the buffer of vertex variables */
} SRPVertexShaderOut;

/** Represents the vertex shader
 *  @see SRPShaderProgram */
typedef struct SRPVertexShader
{
	/** Shader function */
	void (*shader)(SRPVertexShaderIn* in, SRPVertexShaderOut* out);
	/** Number of varyings outputted by this shader */
	size_t nVaryings;
	/** Array of metadata for each varying, should be `nVaryings` elements long */
	SRPVaryingInfo* varyingsInfo;
	/** Total size of all varyings in bytes */
	size_t varyingsSize;
} SRPVertexShader;


/** Holds inputs to fragment shader
 *  @see SRPFragmentShader */
typedef struct SRPFragmentShaderIn
{
	SRPUniform* uniform;        /**< Pointer to currently used shader uniform */
	SRPInterpolated* varyings;  /**< Pointer to interpolated vertex variables */
	float fragCoord[4];         /**< Window space coordinates of the fragment */
	bool frontFacing;           /**< Whether or not the current primitive is facing front */
	size_t primitiveID;         /**< ID of the currently processing primitive */
} SRPFragmentShaderIn;

/** Holds outputs from fragment shader
 *  @see SRPFragmentShader */
typedef struct SRPFragmentShaderOut
{
	float color[4];   /**< Color to draw at this fragment */
	float fragDepth;  /**< Set the depth value of the current fragment. May be written
						   only if SRPFragmentShader.mayOverwriteDepth is `true` */
} SRPFragmentShaderOut;

/** Represents the fragment shader
 *  @see SRPShaderProgram */
typedef struct SRPFragmentShader
{
	/** Shader function */
	void (*shader)(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out);
	/** Whether or not this shader may overwrite depth via SRPFragmentShaderOut.fragDepth
	 *  If `false`, early depth test is activated (less fragment shader invocations) */
	bool mayOverwriteDepth;
} SRPFragmentShader;


/** Holds shaders and a uniform. While not being a program (i.e. is not compiled),
 *  the naming is chosen to be similar to OpenGL's shader program 
 *  @see srpDrawVertexBuffer srpDrawIndexBuffer */
typedef struct SRPShaderProgram
{
	SRPUniform* uniform;    /** Pointer to the uniform to use */
	SRPVertexShader* vs;    /** Pointer to the vertex shader to use */
	SRPFragmentShader* fs;  /** Pointer to the fragment shader to use */
} SRPShaderProgram;

/** @} */  // ingroup Shaders
