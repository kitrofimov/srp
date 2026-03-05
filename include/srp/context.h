// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Context
 *  SRPContext and related functions */

#pragma once

#include <stdbool.h>
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

/** Lists possible compare operations */
typedef enum SRPCompareOp
{
    SRP_COMPARE_NEVER,
    SRP_COMPARE_ALWAYS,
    SRP_COMPARE_LESS,
    SRP_COMPARE_LEQUAL,
    SRP_COMPARE_GREATER,
    SRP_COMPARE_GEQUAL,
    SRP_COMPARE_EQUAL,
    SRP_COMPARE_NOTEQUAL
} SRPCompareOp;


typedef struct SRPRasterState
{
	SRPFrontFace frontFace;      /**< Which face is considered front-facing */
	SRPCullFace cullFace;        /**< Which face(s) should be culled */
	SRPPolygonMode polygonMode;  /**< Polygon rendering mode */
	float pointSize;             /**< Size of rasterized point, in pixels */
} SRPRasterState;

typedef struct SRPDepthState
{
    bool testEnable;
    bool writeEnable;
    SRPCompareOp compareOp;
} SRPDepthState;


/** Holds runtime settings. This always needs to be declared as `SRPContext
 *  srpContext` in user programs and initialized with srpNewContext() */
typedef struct SRPContext
{
	/** Message callback that is called whenever an error/warning/etc. occurs */
	SRPMessageCallback messageCallback;

	/** Which vertex is considered to be the provoking vertex */
	SRPProvokingVertexMode provokingVertexMode;

	SRPRasterState raster;  /** Rasterizer state */
	SRPDepthState depth;    /** Depth test state */

	/** Arena for internal allocations. Is not exposed to the user */
	SRPArena* arena;  
} SRPContext;


/** Initialize the context
 *  @param[in] pContext The pointer to the context */
void srpNewContext(SRPContext* pContext);

/** Set message callback function */
void srpSetMessageCallback(SRPMessageCallback callback);

/** Set the provoking vertex convention */
void srpProvokingVertexMode(SRPProvokingVertexMode mode);

/** Set the face(s) to cull */
void srpRasterCullFace(SRPCullFace face);
/** Set the winding order considered the front face */
void srpRasterFrontFace(SRPFrontFace face);
/** Set polygon rasterization mode */
void srpRasterPolygonMode(SRPPolygonMode mode);
/** Set point size */
void srpRasterPointSize(float size);

/** Enable or disable the depth test */
void srpDepthTest(bool enable);
/** Enable or disable the depth writing */
void srpDepthWrite(bool enable);
/** Set the compare operation used in the depth test */
void srpDepthCompareOp(SRPCompareOp op);


/** Global context declaration */
extern SRPContext srpContext;

/** @} */  // ingroup Context
