// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  SRPContext and related functions */

#pragma once

#include <stddef.h>
#include "message_callback.h"
#include "arena.h"

/** @ingroup Context
 *  @{ */

/** Available attribute interpolation types */
typedef enum SRPInterpolationMode
{
	SRP_INTERPOLATION_MODE_PERSPECTIVE,
	SRP_INTERPOLATION_MODE_AFFINE
} SRPInterpolationMode;

/** Available front face modes */
typedef enum SRPFrontFace
{
	SRP_FRONT_FACE_CCW,
	SRP_FRONT_FACE_CW
} SRPFrontFace;

/** Available cull face modes */
typedef enum SRPCullFace
{
	SRP_CULL_FACE_NONE,
	SRP_CULL_FACE_FRONT,
	SRP_CULL_FACE_BACK,
	SRP_CULL_FACE_FRONT_AND_BACK
} SRPCullFace;

/** Holds runtime settings. This always needs to be declared as `SRPContext
 *  srpContext` in user programs and initialized with srpNewContext() */
typedef struct SRPContext
{
	/** Message callback function that is called whenever an error/warning/etc. occurs */
	SRPMessageCallbackType messageCallback;
	/** User pointer to pass to message callback function
	 *  @see SRPContext.messageCallback */
	void* messageCallbackUserParameter;
	/** How to interpolate vertex attributes inside the primitive */
	SRPInterpolationMode interpolationMode;
	SRPFrontFace frontFace;  /** Which face is considered front-facing */
	SRPCullFace cullFace;    /** Which face(s) are culled */
	/** Arena for internal allocations. Is not exposed to the user */
	SRPArena* arena;
} SRPContext;

/** Possible arguments to `srpContextSet...` */
typedef enum SRPContextParameter
{
	SRP_CONTEXT_MESSAGE_CALLBACK,
	SRP_CONTEXT_MESSAGE_CALLBACK_USER_PARAMETER,
	SRP_CONTEXT_INTERPOLATION_MODE,
	SRP_CONTEXT_FRONT_FACE,
	SRP_CONTEXT_CULL_FACE
} SRPContextParameter;

/** Initialize the context
 *  @param[in] pContext The pointer to context */
void srpNewContext(SRPContext* pContext);

/** Set a pointer parameter in the context
 *  @param[in] contextParameter The context parameter you want to modify
 *  @param[in] data The pointer you want to assign to specified context parameter */
void srpContextSetP(SRPContextParameter contextParameter, void* data);

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

