// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Context_internal
 *  SRPContext implementation */

#include "utils/message_callback_p.h"
#include "srp/context.h"
#include "memory/arena_p.h"

/** @ingroup Context_internal
 *  @{ */

void srpNewContext(SRPContext* pContext)
{
	pContext->messageCallback = (SRPMessageCallback) {
		.func = NULL,
		.userParameter = NULL
	};

	pContext->provokingVertexMode = SRP_PROVOKING_VERTEX_LAST;

	pContext->raster = (SRPRasterState) {
		.frontFace = SRP_FRONT_FACE_CCW,
		.cullFace = SRP_CULL_FACE_NONE,
		.polygonMode = SRP_POLYGON_MODE_FILL,
		.pointSize = 1.
	};
	pContext->depth = (SRPDepthState) {
		.testEnable = false,
		.writeEnable = true,
		.compareOp = SRP_COMPARE_GREATER,  /** @todo use LESS here? like OpenGL */
	};

	pContext->arena = newArena(SRP_DEFAULT_ARENA_CAPACITY);
}

void srpSetMessageCallback(SRPMessageCallback callback)
{
	srpContext.messageCallback = callback;
}

void srpProvokingVertexMode(SRPProvokingVertexMode mode)
{
	srpContext.provokingVertexMode = mode;
}

void srpRasterCullFace(SRPCullFace face)
{
	srpContext.raster.cullFace = face;
}

void srpRasterFrontFace(SRPFrontFace face)
{
	srpContext.raster.frontFace = face;
}

void srpRasterPolygonMode(SRPPolygonMode mode)
{
	srpContext.raster.polygonMode = mode;
}

void srpRasterPointSize(float size)
{
	srpContext.raster.pointSize = size;
}

void srpDepthTest(bool enable)
{
	srpContext.depth.testEnable = enable;
}

void srpDepthWrite(bool enable)
{
	srpContext.depth.writeEnable = enable;
}

void srpDepthCompareOp(SRPCompareOp op)
{
	srpContext.depth.compareOp = op;
}

/** @} */  // ingroup Context_internal
