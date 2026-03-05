#define SRP_INCLUDE_VEC
#define SRP_INCLUDE_MAT

#include <stdio.h>
#include <assert.h>
#include <srp/srp.h>
#include "save.h"
#include "objparser.h"
#include "rad.h"

typedef struct Uniform
{
	size_t frameCount;
	mat4 model;
	mat4 view;
	mat4 projection;
} Uniform;

SRPContext srpContext;

void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out);
void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out);

int main(int argc, char** argv)
{
    assert(argc >= 2);
    const char* outputPath = argv[1];

	OBJMesh mesh;
	if (!loadOBJMesh("res/objects/utah_teapot.obj", &mesh))
	{
		fprintf(stderr, "Failed to load mesh!\n");
		return -1;
	}

	Uniform uniform = {
        .model = mat4ConstructRotate(RAD(-90), 0.2, 0),
		.view = mat4ConstructView(
			0, 1.75, -4,  // Forcing to clip with the near plane
			0, 0, 0,
			1, 1, 1
		),
		.projection = mat4ConstructPerspectiveProjection(-1, 1, -1, 1, 1, 10),
		.frameCount = 0
	};

	SRPShaderProgram shaderProgram = {
		.uniform = (SRPUniform*) &uniform,
		.vs = &(SRPVertexShader) {
			.shader = vertexShader,
			.nVaryings = 0,
			.varyingsInfo = NULL,
			.varyingsSize = 0
		},
		.fs = &(SRPFragmentShader) {
			.shader = fragmentShader,
			.mayOverwriteDepth = false
		}
	};

	srpNewContext(&srpContext);
	srpRasterFrontFace(SRP_FRONT_FACE_CW);
	srpRasterCullFace(SRP_CULL_FACE_BACK);
	srpRasterPolygonMode(SRP_POLYGON_MODE_FILL);

	SRPFramebuffer* fb = srpNewFramebuffer(512, 512);
	SRPVertexBuffer* vb = srpNewVertexBuffer();
	SRPIndexBuffer* ib = srpNewIndexBuffer();
	srpVertexBufferCopyData(vb, sizeof(OBJVertex), mesh.vertexCount * sizeof(OBJVertex), mesh.vertices);
	srpIndexBufferCopyData(ib, SRP_UINT32, mesh.indexCount * sizeof(uint32_t), mesh.indices);

    srpFramebufferClear(fb);
    srpDrawIndexBuffer(ib, vb, fb, &shaderProgram, SRP_PRIM_TRIANGLES, 0, mesh.indexCount);

    int ok = saveFramebufferToImage(fb, outputPath);

	srpFreeVertexBuffer(vb);
	srpFreeIndexBuffer(ib);
	srpFreeFramebuffer(fb);
	freeOBJMesh(&mesh);

    return ok ? 0 : 1;
}


void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out)
{
	OBJVertex* pVertex = (OBJVertex*) in->vertex;
	Uniform* pUniform = (Uniform*) in->uniform;

	vec3* inPosition = &pVertex->position;
	vec4* outPosition = (vec4*) out->clipPosition;
	*outPosition = (vec4) {
		inPosition->x, inPosition->y, inPosition->z, 1.0
	};
	*outPosition = mat4MultiplyVec4(&pUniform->model, *outPosition);
	*outPosition = mat4MultiplyVec4(&pUniform->view, *outPosition);
	*outPosition = mat4MultiplyVec4(&pUniform->projection, *outPosition);
}

void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out)
{
    int id = in->primitiveID;
    int r = (id * 97) % 255;
    int g = (id * 57) % 255;
    int b = (id * 23) % 255;

    out->color[0] = r / 255.;
    out->color[1] = g / 255.;
    out->color[2] = b / 255.;
    out->color[3] = 1.;
}
