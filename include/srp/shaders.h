// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

/** @file
 *  Typedefs related to shaders */

#include <stdbool.h>
#include "srp/vertex.h"

/** @ingroup Shaders
 *  @{ */

/** Represents user's shader uniform
 *  @see SRPShaderProgram */
typedef struct SRPUniform SRPUniform;


/** Holds inputs to vertex shader
 *  @see SRPVertexShader */
typedef struct SRPvsInput
{
	SRPUniform* uniform;  /**< Pointer to currently used shader uniform */
	SRPVertex* pVertex;   /**< Pointer to currently processing vertex */
	size_t vertexID;      /**< ID of a currently processing vertex */
} SRPvsInput;

/** Holds outputs from vertex shader
 *  @see SRPVertexShader */
typedef struct SRPvsOutput
{
	double position[4];                   /**< Position of processed vertex */
	SRPVertexVariable* pOutputVariables;  /**< Pointer to the buffer of vertex variables */
} SRPvsOutput;

/** Represents the vertex shader
 *  @see SRPShaderProgram */
typedef struct SRPVertexShader
{
	/**< Shader function */
	void (*shader)(SRPvsInput* in, SRPvsOutput* out);
	/**< Number of output variables */
	size_t nOutputVariables;
	/**< Pointer to an array of output variables' information.
	 *   Should be SRPVertexShader.nOutputVariables elements long. */
	SRPVertexVariableInformation* outputVariablesInfo;
	/** Size of output variables in bytes */
	size_t nBytesPerOutputVariables;
} SRPVertexShader;


/** Holds inputs to fragment shader
 *  @see SRPFragmentShader */
typedef struct SRPfsInput
{
	SRPUniform* uniform;            /**< Pointer to currently used shader uniform */
	SRPInterpolated* interpolated;  /**< Pointer to interpolated vertex variables */
	double fragCoord[4];            /**< Window space coordinates of the fragment */
	bool frontFacing;               /**< Whether or not the current primitive is facing front */
	size_t primitiveID;             /**< ID of the currently processing primitive */
} SRPfsInput;

/** Holds outputs from fragment shader
 *  @see SRPFragmentShader */
typedef struct SRPfsOutput
{
	double color[4];   /**< Color to draw at this fragment */
	double fragDepth;  /**< Depth value of the current fragment. If not written manually
                            by the user, the fragCoord[2] value is used */
} SRPfsOutput;

/** Represents the fragment shader
 *  @see ShaderProgram */
typedef struct SRPFragmentShader
{
	void (*shader)(SRPfsInput* in, SRPfsOutput* out);  /** Shader function */
} SRPFragmentShader;


/** Holds shaders and a uniform.
 *  While not being a program (is not compiled or anything), the naming is
 *  chosen because it is similar to OpenGL's shader program 
 *  @see srpDrawVertexBuffer srpDrawIndexBuffer */
typedef struct SRPShaderProgram
{
	SRPUniform* uniform;   /** Pointer to uniform to use */

	SRPVertexShader* vs;    /** Pointer to vertex shader to use */
	SRPFragmentShader* fs;  /** Pointer to fragment shader to use */
} SRPShaderProgram;

/** @} */  // defgroup Shaders

