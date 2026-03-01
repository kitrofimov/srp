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
 *  @see SRPvsInput.uniform SRPfsInput.uniform SRPShaderProgram */
typedef struct SRPUniform SRPUniform;


/** Holds inputs to vertex shader
 *  @see SRPVertexShader */
typedef struct SRPvsInput
{
	SRPUniform* uniform;  /**< Pointer to the currently used shader uniform */
	SRPVertex* vertex;   /**< Pointer to the current vertex */
	size_t vertexID;      /**< ID of a current vertex */
} SRPvsInput;

/** Holds outputs from vertex shader
 *  @see SRPVertexShader */
typedef struct SRPvsOutput
{
	float position[4];            /**< Position of the processed vertex */
	SRPVarying* varyings;  /**< Pointer to the buffer of vertex variables */
} SRPvsOutput;

/** Represents the vertex shader
 *  @see SRPShaderProgram */
typedef struct SRPVertexShader
{
	/** Shader function */
	void (*shader)(SRPvsInput* in, SRPvsOutput* out);
	/** Number of varyings outputted by this shader */
	size_t nVaryings;
	/** Array of metadata for each varying, should be `nVaryings` elements long */
	SRPVaryingInfo* varyingsInfo;
	/** Total size of all varyings in bytes */
	size_t varyingsSize;
} SRPVertexShader;


/** Holds inputs to fragment shader
 *  @see SRPFragmentShader */
typedef struct SRPfsInput
{
	SRPUniform* uniform;        /**< Pointer to currently used shader uniform */
	SRPInterpolated* varyings;  /**< Pointer to interpolated vertex variables */
	float fragCoord[4];         /**< Window space coordinates of the fragment */
	bool frontFacing;           /**< Whether or not the current primitive is facing front */
	size_t primitiveID;         /**< ID of the currently processing primitive */
} SRPfsInput;

/** Holds outputs from fragment shader
 *  @see SRPFragmentShader */
typedef struct SRPfsOutput
{
	float color[4];   /**< Color to draw at this fragment */
	float fragDepth;  /**< Set the depth value of the current fragment. May be written
						   only if SRPFragmentShader.mayOverwriteDepth is `true` */
} SRPfsOutput;

/** Represents the fragment shader
 *  @see SRPShaderProgram */
typedef struct SRPFragmentShader
{
	/** Shader function */
	void (*shader)(SRPfsInput* in, SRPfsOutput* out);
	/** Whether or not this shader may overwrite depth via SRPfsOutput.fragDepth
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
