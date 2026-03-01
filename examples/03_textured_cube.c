#define SRP_INCLUDE_VEC
#define SRP_INCLUDE_MAT

#include <stdio.h>
#include <srp/srp.h>
#include "window.h"
#include "framelimiter.h"

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

void messageCallback(
	SRPMessageType type, SRPMessageSeverity severity, const char* sourceFunction,
	const char* message, void* userParameter
);
void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out);
void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out);

int main()
{
	srpNewContext(&srpContext);
	srpContextSetMessageCallback(messageCallback);

	// Enable back-face culling and set counter-clockwise faces as front-facing
	srpContextSetI(SRP_CONTEXT_FRONT_FACE, SRP_FRONT_FACE_CCW);
	srpContextSetI(SRP_CONTEXT_CULL_FACE, SRP_CULL_FACE_BACK);

	SRPFramebuffer* fb = srpNewFramebuffer(512, 512);

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

	// Create vertex and index buffers, these are similar to VBO and EBO
	SRPVertexBuffer* vb = srpNewVertexBuffer();
	SRPIndexBuffer* ib = srpNewIndexBuffer();
	srpVertexBufferCopyData(vb, sizeof(Vertex), sizeof(data), data);
	srpIndexBufferCopyData(ib, SRP_UINT8, sizeof(indices), indices);

	Uniform uniform = {
		.model = mat4ConstructIdentity(),
		.view = mat4ConstructView(
			0, 0, -3,
			0, 0, 0,
			1, 1, 1
		),
		.projection = mat4ConstructPerspectiveProjection(-1, 1, -1, 1, 1, 50),
		.texture = srpNewTexture("./res/textures/stoneWall.png", TW_REPEAT, TW_REPEAT),
		.frameCount = 0
	};

	SRPShaderProgram shaderProgram = {
		.uniform = (SRPUniform*) &uniform,
		.vs = &(SRPVertexShader) {
			.shader = vertexShader,
			.nVaryings = 1,
			.varyingsInfo = (SRPVaryingInfo[]) {{
				.nItems = 2,
				.type = SRP_FLOAT,
				.interpolationMode = SRP_INTERPOLATION_MODE_PERSPECTIVE
			}},
			.varyingsSize = sizeof(VSOutput)
		},
		.fs = &(SRPFragmentShader) {
			.shader = fragmentShader,
			.mayOverwriteDepth = false
		}
	};

	Window* window = newWindow(512, 512, "Rasterizer", false);
	FrameLimiter limiter;
	frameLimiterInit(&limiter, 144.);

	while (window->running)
	{
		frameLimiterBegin(&limiter);

		float renderTime = 0.;
		TIME_SECTION(renderTime, {
			uniform.model = mat4ConstructRotate(
				uniform.frameCount / 100.,
				uniform.frameCount / 200.,
				uniform.frameCount / 500.
			);

			srpFramebufferClear(fb);
			srpDrawIndexBuffer(ib, vb, fb, &shaderProgram, SRP_PRIM_TRIANGLES, 0, 36);
		});

		windowPollEvents(window);
		windowPresent(window, fb);

		float frameTime = frameLimiterEnd(&limiter);
		uniform.frameCount++;

		if (uniform.frameCount % 100 == 0)
			printf(
				"Frametime: %5.3f ms; Rendering: %5.3f ms; FPS: %6.2f; RPS: %6.2f\n",
				frameTime * 1000., renderTime * 1000., 1. / frameTime, 1. / renderTime
			);
	}

	srpFreeTexture(uniform.texture);
	srpFreeVertexBuffer(vb);
	srpFreeIndexBuffer(ib);
	srpFreeFramebuffer(fb);
	freeWindow(window);

	return 0;
}


void messageCallback(
	SRPMessageType type, SRPMessageSeverity severity, const char* sourceFunction,
	const char* message, void* userParameter
)
{
	fprintf(stderr, "%s: %s", sourceFunction, message);
}


void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out)
{
	Vertex* pVertex = (Vertex*) in->vertex;
	Uniform* pUniform = (Uniform*) in->uniform;
	VSOutput* pOutVars = (VSOutput*) out->varyings;

	vec3* inPosition = &pVertex->position;
	vec4* outPosition = (vec4*) out->clipPosition;
	*outPosition = (vec4) { inPosition->x, inPosition->y, inPosition->z, 1. };
	*outPosition = mat4MultiplyVec4(&pUniform->model, *outPosition);
	*outPosition = mat4MultiplyVec4(&pUniform->view, *outPosition);
	*outPosition = mat4MultiplyVec4(&pUniform->projection, *outPosition);

	pOutVars->uv.x = pVertex->uv.x;
	pOutVars->uv.y = pVertex->uv.y;
}

void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out)
{
	VSOutput* interpolated = (VSOutput*) in->varyings;
	Uniform* pUniform = (Uniform*) in->uniform;
	vec3* outColor = (vec3*) out->color;

	vec2 uv = interpolated->uv;
	srpTextureGetFilteredColor(pUniform->texture, uv.x, uv.y, (float*) outColor);
}

