// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Context
 *  SRPContext and related functions */

#pragma once

#include <stddef.h>
#include <stdint.h>
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
	SRP_FRONT_FACE_CCW,  /**< Counterclockwise (default) */
	SRP_FRONT_FACE_CW    /**< Clockwise */
} SRPFrontFace;

/** Cull face mode */
typedef enum SRPCullFace
{
	SRP_CULL_FACE_NONE,   /**< Do not cull any face (default) */
	SRP_CULL_FACE_FRONT,  /**< Cull the front face */
	SRP_CULL_FACE_BACK,   /**< Cull the back face */
	/**< Cull both front and back faces. Primitives that don't have a face
	 *   (lines, points) are left as-is. */
	SRP_CULL_FACE_FRONT_AND_BACK
} SRPCullFace;

/** Polygon rendering mode */
typedef enum SRPPolygonMode
{
	SRP_POLYGON_MODE_FILL,  /**< Filled triangles (default) */
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

/** Lists possible stencil buffer operations */
typedef enum {
    SRP_STENCIL_KEEP,       /**< Keep the current value */
    SRP_STENCIL_ZERO,       /**< Set to 0 */
    SRP_STENCIL_REPLACE,    /**< Set to the reference value */
    SRP_STENCIL_INCR,       /**< Increment (clamp to 255) */
    SRP_STENCIL_INCR_WRAP,  /**< Increment (wraparound to 0) */
    SRP_STENCIL_DECR,       /**< Decrement (clamp to 0) */
    SRP_STENCIL_DECR_WRAP,  /**< Decrement (wraparound to 255) */
    SRP_STENCIL_INVERT      /**< Bitwise invert */
} SRPStencilOp;



/** Rasterizer state */
typedef struct SRPRasterState
{
	SRPFrontFace frontFace;      /**< Which face is considered front-facing */
	SRPCullFace cullFace;        /**< Which face(s) should be culled */
	SRPPolygonMode polygonMode;  /**< Polygon rendering mode */
	float pointSize;             /**< Size of rasterized point, in pixels */
} SRPRasterState;

/** Scissor test state */
typedef struct {
    bool enabled;   /**< Whether or not the scissor test is enabled */
    size_t x;       /**< The x position of the scissor box's upper left corner */
    size_t y;       /**< The y position of the scissor box's upper left corner */
    size_t width;   /**< The width of the scissor box */
    size_t height;  /**< The height of the scissor box */
} SRPScissorState;

/** Stencil test state */
typedef struct {
    bool enabled;          /**< Whether or not the stencil test is enabled */
    SRPCompareOp func;     /**< The comparison function to use */
    uint8_t ref;           /**< The value to compare against */
    uint8_t mask;          /**< Mask for the comparison: (ref & mask) OP (stored & mask) */
    uint8_t writeMask;     /**< Mask for writing to the buffer */
    SRPStencilOp sfailOp;  /**< Action if stencil test fails */
    SRPStencilOp dfailOp;  /**< Action if stencil test passes but depth test fails */
    SRPStencilOp passOp;   /**< Action if both stencial and depth tests pass */
} SRPStencilState;

/** Depth test state */
typedef struct SRPDepthState
{
    bool testEnable;         /**< Whether or not the depth test is enabled */
    bool writeEnable;        /**< Whether or not the depth write is enabled.
								  Does not have any effect if the test is disabled */
    SRPCompareOp compareOp;  /**< Compare operation to use in depth test */
} SRPDepthState;


/** Holds runtime settings. This always needs to be declared as `SRPContext
 *  srpContext` in user programs and initialized with srpNewContext() */
typedef struct SRPContext
{
	/** Message callback that is called whenever an error/warning/etc. occurs */
	SRPMessageCallback messageCallback;

	/** Which vertex is considered to be the provoking vertex */
	SRPProvokingVertexMode provokingVertexMode;

	SRPRasterState raster;    /**< Rasterizer state */
	SRPScissorState scissor;  /**< Scissor test state */
    SRPStencilState stencil;  /**< Stencil test state */
	SRPDepthState depth;      /**< Depth test state */

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

/** Enable or disable the scissor test */
void srpScissorTest(bool enable);
/** Set the options for the scissor test */
void srpScissorOptions(size_t x, size_t y, size_t width, size_t height);

/** Enable or disable the depth test */
void srpDepthTest(bool enable);
/** Enable or disable the depth writing */
void srpDepthWrite(bool enable);
/** Set the compare operation used in the depth test */
void srpDepthCompareOp(SRPCompareOp op);


/** Global context declaration */
extern SRPContext srpContext;

/** @} */  // ingroup Context
