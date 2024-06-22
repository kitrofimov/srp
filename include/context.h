// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  SRPContext and related functions */

#pragma once

#include <stddef.h>
#include "message_callback.h"

/** @ingroup Context
 *  @{ */

/** What face to cull @see SRPFrontFace */
typedef enum SRPCullFace
{
	SRP_CULL_FACE_BACK,            /** Cull only the back face. Default */
	SRP_CULL_FACE_FRONT,           /** Cull only the front face */
	SRP_CULL_FACE_FRONT_AND_BACK   /** Cull both back and front face */
} SRPCullFace;

/** What face to consider the "front" one */
typedef enum SRPFrontFace
{
	/** If vertex order is counterclockwise, consider this a front face. Default */
	SRP_FRONT_FACE_COUNTERCLOCKWISE,
	/** If vertex order is clockwise, consider this a front face */
	SRP_FRONT_FACE_CLOCKWISE
} SRPFrontFace;

/** Available attribute interpolation types */
typedef enum SRPInterpolationMode
{
	SRP_INTERPOLATION_MODE_PERSPECTIVE,
	SRP_INTERPOLATION_MODE_AFFINE
} SRPInterpolationMode;

/** Holds runtime settings. This always needs to be declared as `SRPContext
 *  srpContext` in user programs and initialized with srpNewContext() */
typedef struct SRPContext
{
	/** Message callback function that is called whenever an
	 *  error/warning/etc. occurs */
	SRPMessageCallbackType messageCallback;
	/** User pointer to pass to message callback function
	 *  @see SRPContext.messageCallback */
	void* messageCallbackUserParameter;
	
	/** How to interpolate vertex attributes inside the primitive
	 *  @todo Deprecated! Should make something like this:
	 *  https://www.khronos.org/opengl/wiki/Type_Qualifier_(GLSL)#Interpolation_qualifiers */
	SRPInterpolationMode interpolationMode;

	/** What face to cull */
	SRPCullFace cullFace;
	/** What face to consider the "front" one */
	SRPFrontFace frontFace;
} SRPContext;

/** Possible arguments to `srpContextSet...` */
typedef enum SRPContextParameter
{
	SRP_CONTEXT_MESSAGE_CALLBACK,
	SRP_CONTEXT_MESSAGE_CALLBACK_USER_PARAMETER,

	SRP_CONTEXT_INTERPOLATION_MODE,

	SRP_CONTEXT_CULL_FACE,
	SRP_CONTEXT_FRONT_FACE
} SRPContextParameter;

/** Initialize the context
 *  @param[in] pContext The pointer to context */
void srpNewContext(SRPContext* pContext);

/** Set a pointer parameter in the context
 *  @param[in] contextParameter The context parameter you want to modify
 *  @param[in] data The pointer you want to assign to specified context parameter */
void srpContextSetP(SRPContextParameter contextParameter, const void* data);

/** Set an integer/enum parameter in the context
 *  @param[in] contextParameter The context parameter you want to modify
 *  @param[in] data The value you want to assign to specified context parameter */
void srpContextSetI(SRPContextParameter contextParameter, int data);

/** Get a pointer parameter in the context
 *  @param[in] contextParameter The context parameter you want to get
 *  @return Requested parameter or NULL on error */
void* srpContextGetP(SRPContextParameter contextParameter);

/** Get an integer/enum parameter in the context
 *  @param[in] contextParameter The context parameter you want to get
 *  @return Requested parameter or 0 on error */
int srpContextGetI(SRPContextParameter contextParameter);

/** @} */  // defgroup Context

extern SRPContext srpContext;

