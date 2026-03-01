// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Context
 *  SRPContext and related functions */

#pragma once

#include <stddef.h>
#include "srp/message_callback.h"
#include "srp/arena.h"

/** @ingroup Context
 *  @{ */

/** Which vertex is considered to be the provoking vertex */
typedef enum SRPProvokingVertexMode
{
	/** The first vertex of the primitive is its provoking vertex */
	SRP_PROVOKING_VERTEX_FIRST,  
	/** The last vertex of the primitive is its provoking vertex (default) */
	SRP_PROVOKING_VERTEX_LAST
} SRPProvokingVertexMode;

/** Front face mode */
typedef enum SRPFrontFace
{
	SRP_FRONT_FACE_CCW,  /**< Counterclockwise; default */
	SRP_FRONT_FACE_CW    /**< Clockwise */
} SRPFrontFace;

/** Cull face mode */
typedef enum SRPCullFace
{
	SRP_CULL_FACE_NONE,   /**< Do not cull any face; default */
	SRP_CULL_FACE_FRONT,  /**< Cull the front face */
	SRP_CULL_FACE_BACK,   /**< Cull the back face */
	/**< Cull both front and back faces. Primitives that don't have a face
	 *   (lines, points) are left as-is. */
	SRP_CULL_FACE_FRONT_AND_BACK
} SRPCullFace;

/** Polygon rendering mode */
typedef enum SRPPolygonMode
{
	SRP_POLYGON_MODE_FILL,  /**< Filled triangles; default */
	SRP_POLYGON_MODE_LINE,  /**< Lines only (wireframe) */
	SRP_POLYGON_MODE_POINT  /**< Points only */
} SRPPolygonMode;

/** Holds runtime settings. This always needs to be declared as `SRPContext
 *  srpContext` in user programs and initialized with srpNewContext() */
typedef struct SRPContext
{
	/** Message callback function that is called whenever an error/warning/etc. occurs */
	SRPMessageCallbackType messageCallback;
	/** User pointer to pass to message callback function
	 *  @see SRPContext.messageCallback */
	void* messageCallbackUserParameter;
	/** Which vertex is considered to be the provoking vertex */
	SRPProvokingVertexMode provokingVertexMode;
	SRPFrontFace frontFace;      /**< Which face is considered front-facing */
	SRPCullFace cullFace;        /**< Which face(s) should be culled */
	SRPPolygonMode polygonMode;  /**< Polygon rendering mode */
	float pointSize;             /**< Size of rasterized point, in pixels */

	SRPArena* arena;  /**< Arena for internal allocations. Is not exposed to the user */
} SRPContext;

/** Possible arguments to srpContextSetP(), srpContextSetI(), srpContextSetF() */
typedef enum SRPContextParameter
{
	SRP_CONTEXT_MESSAGE_CALLBACK_USER_PARAMETER,
	SRP_CONTEXT_PROVOKING_VERTEX_MODE,
	SRP_CONTEXT_FRONT_FACE,
	SRP_CONTEXT_CULL_FACE,
	SRP_CONTEXT_POLYGON_MODE,
	SRP_CONTEXT_POINT_SIZE,
} SRPContextParameter;

/** Initialize the context
 *  @param[in] pContext The pointer to context */
void srpNewContext(SRPContext* pContext);

/** Set message callback function
 *  @param[in] callback The pointer to the message callback function */
void srpContextSetMessageCallback(SRPMessageCallbackType callback);

/** Get the current message callback function
 *  @return The pointer to the current message callback function */
SRPMessageCallbackType srpContextGetMessageCallback();

/** Set a pointer parameter in the context
 *  @param[in] contextParameter The context parameter you want to modify
 *  @param[in] data The pointer you want to assign to specified context parameter */
void srpContextSetP(SRPContextParameter contextParameter, void* data);

/** Set an integer/enum parameter in the context
 *  @param[in] contextParameter The context parameter you want to modify
 *  @param[in] data The value you want to assign to specified context parameter */
void srpContextSetI(SRPContextParameter contextParameter, int data);

/** Set a float parameter in the context
 *  @param[in] contextParameter The context parameter you want to modify
 *  @param[in] data The value you want to assign to specified context parameter */
void srpContextSetF(SRPContextParameter contextParameter, float data);

/** Get a pointer parameter in the context
 *  @param[in] contextParameter The context parameter you want to get
 *  @return Requested parameter or NULL on error */
void* srpContextGetP(SRPContextParameter contextParameter);

/** Get an integer/enum parameter in the context
 *  @param[in] contextParameter The context parameter you want to get
 *  @return Requested parameter or 0 on error */
int srpContextGetI(SRPContextParameter contextParameter);

/** Get a float parameter in the context
 *  @param[in] contextParameter The context parameter you want to get
 *  @return Requested parameter or 0 on error */
float srpContextGetF(SRPContextParameter contextParameter);

/** Global context declaration */
extern SRPContext srpContext;

/** @} */  // ingroup Context
