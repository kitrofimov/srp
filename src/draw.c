// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Draw dispatch implementation */

#include "draw.h"
#include "triangle.h"
#include "message_callback_p.h"
#include "context.h"
#include "primitive_assembly.h"
#include "arena_p.h"

/** Draw triangle-based primitives from either SRPIndexBuffer or SRPVertexBuffer
 *  @see drawBuffer() for extensive documentation */
static void drawTriangles(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
);

/** Draw line-based primitives from either SRPIndexBuffer or SRPVertexBuffer
 *  @see drawBuffer() for extensive documentation */
static void drawLines(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
);

/** Draw points from either SRPIndexBuffer or SRPVertexBuffer
 *  @see drawBuffer() for extensive documentation */
static void drawPoints(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
);

/** Check if the draw call tries to access out-of-bounds memory and
 * 	if so, send an error message
 *  @see drawBuffer() for extensive documentation */
static bool checkOOB(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb,
	size_t startIndex, size_t count
);

/** Determine if a primitive is triangle-based
 *  @param[in] primitive Primitive type
 *  @return `true` if the primitive is triangle-based, `false` otherwise */
static bool isPrimitiveTriangle(SRPPrimitive primitive);

/** Determine if a primitive is line-based
 *  @param[in] primitive Primitive type
 *  @return `true` if the primitive is line-based, `false` otherwise */
static bool isPrimitiveLine(SRPPrimitive primitive);

/** Determine if a primitive is a point
 *  @param[in] primitive Primitive type
 *  @return `true` if the primitive is a point, `false` otherwise */
static bool isPrimitivePoint(SRPPrimitive primitive);

void drawBuffer(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
)
{
	if (count == 0 || checkOOB(ib, vb, startIndex, count))
		return;

	if (isPrimitiveTriangle(primitive))
		drawTriangles(ib, vb, fb, sp, primitive, startIndex, count);
	else if (isPrimitiveLine(primitive))
		drawLines(ib, vb, fb, sp, primitive, startIndex, count);
	else if (isPrimitivePoint(primitive))
		drawPoints(ib, vb, fb, sp, primitive, startIndex, count);
	else
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Unknown primitive type: %i", primitive
		);
}

static void drawTriangles(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
)
{
	if (srpContext.cullFace == SRP_CULL_FACE_FRONT_AND_BACK)
		return;

	size_t triangleCount;
	SRPTriangle* triangles;
	bool success = assembleTriangles(
		ib, vb, fb, sp, primitive, startIndex, count,
		&triangleCount, &triangles
	);
	if (!success)
		return;

	void* interpolatedBuffer = ARENA_ALLOC(sp->vs->nBytesPerOutputVariables);
	for (size_t i = 0; i < triangleCount; i++)
		rasterizeTriangle(&triangles[i], fb, sp, interpolatedBuffer);

	ARENA_RESET();
}

static void drawLines(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
)
{
	size_t lineCount;
	SRPLine* lines;
	bool success = assembleLines(
		ib, vb, fb, sp, primitive, startIndex, count,
		&lineCount, &lines
	);
	if (!success)
		return;

	void* interpolatedBuffer = ARENA_ALLOC(sp->vs->nBytesPerOutputVariables);
	for (size_t i = 0; i < lineCount; i++)
		rasterizeLine(&lines[i], fb, sp, interpolatedBuffer);

	ARENA_RESET();
}

static void drawPoints(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb, const SRPFramebuffer* fb,
	const SRPShaderProgram* sp, SRPPrimitive primitive, size_t startIndex, size_t count
)
{
	size_t pointCount;
	SRPPoint* points;
	bool success = assemblePoints(
		ib, vb, fb, sp, startIndex, count, &pointCount, &points
	);
	if (!success)
		return;

	for (size_t i = 0; i < pointCount; i++)
		rasterizePoint(&points[i], fb, sp);

	ARENA_RESET();
}

static bool checkOOB(
	const SRPIndexBuffer* ib, const SRPVertexBuffer* vb,
	size_t startIndex, size_t count
)
{
	// The last stream index that will be touched
	const size_t endIndex = startIndex + count - 1;
	const size_t bufferSize = (ib) ? ib->nIndices : vb->nVertices;

	if (endIndex >= bufferSize)
	{
		const char* errorMessage = (ib) ? \
			"Attempt to OOB access index buffer (read) at indices %zu-%zu (size: %zu)\n" : \
			"Attempt to OOB access vertex buffer (read) at indices %zu-%zu (size: %zu)\n";
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			errorMessage, startIndex, endIndex, bufferSize
		);
		return true;
	}

	return false;
}

static bool isPrimitiveTriangle(SRPPrimitive primitive)
{
	return primitive == SRP_PRIM_TRIANGLES ||
		primitive == SRP_PRIM_TRIANGLE_STRIP ||
		primitive == SRP_PRIM_TRIANGLE_FAN;
}

static bool isPrimitiveLine(SRPPrimitive primitive)
{
	return primitive == SRP_PRIM_LINES ||
		primitive == SRP_PRIM_LINE_STRIP ||
		primitive == SRP_PRIM_LINE_LOOP;
}

static bool isPrimitivePoint(SRPPrimitive primitive)
{
	return primitive == SRP_PRIM_POINTS;
}
