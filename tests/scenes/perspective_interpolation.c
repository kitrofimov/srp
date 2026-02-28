#define SRP_INCLUDE_VEC
#define SRP_INCLUDE_MAT

#include <assert.h>
#include <stdio.h>
#include <srp/srp.h>
#include "save.h"

typedef struct Vertex
{
	vec3 position;
	vec2 uv;
} Vertex;

typedef struct VSOutput
{
	vec2 uv;
} VSOutput;

typedef struct Uniform
{
	size_t frameCount;
	mat4 model;
	mat4 view;
	mat4 projection;
	SRPTexture* texture;
} Uniform;

SRPContext srpContext;

void vertexShader(SRPvsInput* in, SRPvsOutput* out);
void fragmentShader(SRPfsInput* in, SRPfsOutput* out);

int main(int argc, char** argv)
{
    assert(argc >= 2);
    const char* outputPath = argv[1];

	Vertex data[] = {
		{.position = {-1, -1, -1}, .uv = {0, 0}},
		{.position = { 1, -1, -1}, .uv = {1, 0}},
		{.position = { 1,  1, -1}, .uv = {1, 1}},
		{.position = {-1,  1, -1}, .uv = {0, 1}},

		{.position = {-1,  1, -1}, .uv = {0, 0}},
		{.position = { 1,  1, -1}, .uv = {1, 0}},
		{.position = { 1,  1,  1}, .uv = {1, 1}},
		{.position = {-1,  1,  1}, .uv = {0, 1}},

		{.position = { 1, -1,  1}, .uv = {0, 0}},
		{.position = {-1, -1,  1}, .uv = {1, 0}},
		{.position = {-1,  1,  1}, .uv = {1, 1}},
		{.position = { 1,  1,  1}, .uv = {0, 1}},

		{.position = { 1, -1,  1}, .uv = {0, 0}},
		{.position = { 1, -1, -1}, .uv = {1, 0}},
		{.position = { 1,  1, -1}, .uv = {1, 1}},
		{.position = { 1,  1,  1}, .uv = {0, 1}},

		{.position = {-1, -1, -1}, .uv = {0, 0}},
		{.position = {-1, -1,  1}, .uv = {1, 0}},
		{.position = {-1,  1,  1}, .uv = {1, 1}},
		{.position = {-1,  1, -1}, .uv = {0, 1}},
		
		{.position = {-1, -1, -1}, .uv = {0, 0}},
		{.position = { 1, -1, -1}, .uv = {1, 0}},
		{.position = { 1, -1,  1}, .uv = {1, 1}},
		{.position = {-1, -1,  1}, .uv = {0, 1}}
	};

	uint8_t indices[] = {
		 0,  1,  2,   0,  2,  3,
		 4,  5,  6,   4,  6,  7,
		 8,  9, 10,   8, 10, 11,
		12, 15, 14,  12, 14, 13,
		16, 18, 17,  16, 19, 18,
		20, 23, 22,  20, 22, 21
	};

	Uniform uniform = {
		.model = mat4ConstructRotate(2.5, 0.7, 0.5),
		.view = mat4ConstructView(0, 0, -3,   0, 0, 0,   1, 1, 1),
		.projection = mat4ConstructPerspectiveProjection(-1, 1, -1, 1, 1, 50),
		.texture = srpNewTexture("./res/textures/stoneWall.png", TW_REPEAT, TW_REPEAT),
		.frameCount = 0
	};

	SRPShaderProgram shaderProgram = {
		.uniform = (SRPUniform*) &uniform,
		.vs = &(SRPVertexShader) {
			.shader = vertexShader,
			.nOutputVariables = 1,
			.outputVariablesInfo = (SRPVertexVariableInformation[])	{
				{.nItems = 2, .type = TYPE_FLOAT}
			},
			.nBytesPerOutputVariables = sizeof(VSOutput)
		},
		.fs = &(SRPFragmentShader) {
			.shader = fragmentShader,
			.doesOverwriteDepth = false
		}
	};

	srpNewContext(&srpContext);
	srpContextSetI(SRP_CONTEXT_FRONT_FACE, SRP_FRONT_FACE_CCW);
	srpContextSetI(SRP_CONTEXT_CULL_FACE, SRP_CULL_FACE_BACK);

	SRPFramebuffer* fb = srpNewFramebuffer(512, 512);
	SRPVertexBuffer* vb = srpNewVertexBuffer();
	SRPIndexBuffer* ib = srpNewIndexBuffer();
	srpVertexBufferCopyData(vb, sizeof(Vertex), sizeof(data), data);
	srpIndexBufferCopyData(ib, TYPE_UINT8, sizeof(indices), indices);

    srpFramebufferClear(fb);
    srpDrawIndexBuffer(ib, vb, fb, &shaderProgram, SRP_PRIM_TRIANGLES, 0, 36);

    int ok = saveFramebufferToImage(fb, outputPath);

	srpFreeTexture(uniform.texture);
	srpFreeVertexBuffer(vb);
	srpFreeIndexBuffer(ib);
	srpFreeFramebuffer(fb);

    return ok ? 0 : 1;
}


void vertexShader(SRPvsInput* in, SRPvsOutput* out)
{
	Vertex* pVertex = (Vertex*) in->pVertex;
	Uniform* pUniform = (Uniform*) in->uniform;
	VSOutput* pOutVars = (VSOutput*) out->pOutputVariables;

	vec3* inPosition = &pVertex->position;
	vec4* outPosition = (vec4*) out->position;
	*outPosition = (vec4) {
		inPosition->x, inPosition->y, inPosition->z, 1.0
	};
	*outPosition = mat4MultiplyVec4(&pUniform->model, *outPosition);
	*outPosition = mat4MultiplyVec4(&pUniform->view, *outPosition);
	*outPosition = mat4MultiplyVec4(&pUniform->projection, *outPosition);

	pOutVars->uv.x = pVertex->uv.x;
	pOutVars->uv.y = pVertex->uv.y;
}

void fragmentShader(SRPfsInput* in, SRPfsOutput* out)
{
	VSOutput* interpolated = (VSOutput*) in->interpolated;
	Uniform* pUniform = (Uniform*) in->uniform;
	vec3* outColor = (vec3*) out->color;

	vec2 uv = interpolated->uv;
	srpTextureGetFilteredColor(pUniform->texture, uv.x, uv.y, (float*) outColor);
}
