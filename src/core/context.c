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
		.frontFace = SRP_WINDING_CCW,
		.cullFace = SRP_FACE_NONE,
		.polygonMode = SRP_POLYGON_MODE_FILL,
		.pointSize = 1.
	};
	pContext->scissor = (SRPScissorState) { 0 };
	pContext->depth = (SRPDepthState) {
		.testEnable = false,
		.writeEnable = true,
		.compareOp = SRP_COMPARE_GREATER,  /** @todo use LESS here? like OpenGL */
	};

	SRPStencilFaceState temp = {
		.func = SRP_COMPARE_ALWAYS,
		.ref = 0,
		.mask = 0xFF,
		.writeMask = 0xFF,
		.sfailOp = SRP_STENCIL_KEEP,
		.dfailOp = SRP_STENCIL_KEEP,
		.passOp  = SRP_STENCIL_KEEP
	};
	pContext->stencil.enabled = false;
	pContext->stencil.front = temp;
	pContext->stencil.back = temp;

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

void srpRasterCullFace(SRPFace face)
{
	srpContext.raster.cullFace = face;
}

void srpRasterFrontFace(SRPWinding face)
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

void srpScissorTest(bool enable)
{
	srpContext.scissor.enabled = enable;
}

void srpScissorOptions(size_t x, size_t y, size_t width, size_t height)
{
	srpContext.scissor.x = x;
	srpContext.scissor.y = y;
	srpContext.scissor.width = width;
	srpContext.scissor.height = height;
}

void srpStencilTest(bool enable)
{
	srpContext.stencil.enabled = true;
}

void srpStencilFunc(SRPCompareOp func, uint8_t ref, uint8_t mask)
{
	srpContext.stencil.front.func = func;
	srpContext.stencil.front.ref = ref;
	srpContext.stencil.front.mask = mask;
	srpContext.stencil.back.func = func;
	srpContext.stencil.back.ref = ref;
	srpContext.stencil.back.mask = mask;
}

void srpStencilFuncSeparate(SRPFace face, SRPCompareOp func, uint8_t ref, uint8_t mask)
{
	if (face == SRP_FACE_NONE)
		return;

	if (face == SRP_FACE_FRONT_AND_BACK)
		srpStencilFunc(func, ref, mask);

	SRPStencilFaceState* state = (face == SRP_FACE_FRONT) ? &srpContext.stencil.front : &srpContext.stencil.back;
	state->func = func;
	state->ref = ref;
	state->mask = mask;
}

void srpStencilOp(SRPStencilOp sfail, SRPStencilOp dfail, SRPStencilOp pass)
{
	srpContext.stencil.front.sfailOp = sfail;
	srpContext.stencil.front.dfailOp = dfail;
	srpContext.stencil.front.passOp = pass;
	srpContext.stencil.back.sfailOp = sfail;
	srpContext.stencil.back.dfailOp = dfail;
	srpContext.stencil.back.passOp = pass;
}

void srpStencilOpSeparate(SRPFace face, SRPStencilOp sfail, SRPStencilOp dfail, SRPStencilOp pass)
{
	if (face == SRP_FACE_NONE)
		return;

	if (face == SRP_FACE_FRONT_AND_BACK)
		srpStencilOp(sfail, dfail, pass);

	SRPStencilFaceState* state = (face == SRP_FACE_FRONT) ? &srpContext.stencil.front : &srpContext.stencil.back;
	state->sfailOp = sfail;
	state->dfailOp = dfail;
	state->passOp = pass;
}

void srpStencilWriteMask(uint8_t mask)
{
	srpContext.stencil.front.writeMask = mask;
	srpContext.stencil.back.writeMask = mask;
}

void srpStencilWriteMaskSeparate(SRPFace face, uint8_t mask)
{
	if (face == SRP_FACE_NONE)
		return;

	if (face == SRP_FACE_FRONT_AND_BACK)
		srpStencilWriteMask(mask);

	SRPStencilFaceState* state = (face == SRP_FACE_FRONT) ? &srpContext.stencil.front : &srpContext.stencil.back;
	state->writeMask = mask;
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
