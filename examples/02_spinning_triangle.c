#define SRP_INCLUDE_VEC
#define SRP_INCLUDE_MAT

#include <stdio.h>
#include <srp/srp.h>
#include "window.h"
#include "framelimiter.h"
#include "rad.h"

typedef struct Vertex
{
	vec3 position;
	vec3 color;
} Vertex;

typedef struct VSOutput
{
	vec3 color;
} VSOutput;

// A structure to hold the uniform for shaders
// Can be used to pass arbitrary data to the shader
typedef struct Uniform
{
	size_t frameCount;
	mat4 rotation;
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

	SRPFramebuffer* fb = srpNewFramebuffer(512, 512);

	const float R = 0.8;
	Vertex data[3] = {
		{.position = {0., R, 0.}, .color = {1., 0., 0.}},
		{
			.position = {-cos(RAD(30)) * R, -sin(RAD(30)) * R, 0.},
			.color = {0., 0., 1.}
		},
		{
			.position = { cos(RAD(30)) * R, -sin(RAD(30)) * R, 0.},
			.color = {0., 1., 0.}
		}
	};

	SRPVertexBuffer* vb = srpNewVertexBuffer();
	srpVertexBufferCopyData(vb, sizeof(Vertex), sizeof(data), data);

	// Uniform requires a cast to an opaque `SRPUniform` type to avoid
	// a compiler warning
	Uniform uniform = {0};
	SRPShaderProgram shaderProgram = {
		.uniform = (SRPUniform*) &uniform,
		.vs = &(SRPVertexShader) {
			.shader = vertexShader,
			.nVaryings = 1,
			.varyingsInfo = (SRPVaryingInfo[]) {
				{.nItems = 3, .type = SRP_FLOAT}
			},
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
			// Part of the `mat` API. Again, you can use your own functions
			// (or an external math library) if you remove the `define`s at the
			// top of this file (`SRP_INCLUDE_...`)
			uniform.rotation = mat4ConstructRotate(0, 0, uniform.frameCount / 1000.);
			srpFramebufferClear(fb);
			srpDrawVertexBuffer(vb, fb, &shaderProgram, SRP_PRIM_TRIANGLES, 0, 3);
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

	srpFreeVertexBuffer(vb);
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
	vec4* outPosition = (vec4*) out->position;
	*outPosition = (vec4) {
		inPosition->x, inPosition->y, inPosition->z, 1.0
	};
	// Transform the position vector by the rotation matrix from uniform
	*outPosition = mat4MultiplyVec4(&pUniform->rotation, *outPosition);

	// Transform the color values just for fun
	pOutVars->color.x = pVertex->color.x + sin(pUniform->frameCount * 2.5e-3) * 0.3;
	pOutVars->color.y = pVertex->color.y + sin(pUniform->frameCount * 0.5e-3) * 0.1;
	pOutVars->color.z = pVertex->color.z + sin(pUniform->frameCount * 5e-3) * 0.5;
}

void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out)
{
	VSOutput* interpolated = (VSOutput*) in->varyings;
	vec4* outColor = (vec4*) out->color;
	outColor->x = interpolated->color.x;
	outColor->y = interpolated->color.y;
	outColor->z = interpolated->color.z;
	outColor->w = 1.;
}

