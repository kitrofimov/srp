#define SRP_INCLUDE_VEC
#define SRP_INCLUDE_MAT

#include <stdio.h>
#include <srp/srp.h>
#include "window.h"
#include "timer.h"
#include "rad.h"

typedef struct Vertex
{
	vec3d position;
	vec3d color;
} Vertex;

typedef struct VSOutput
{
	vec3d color;
} VSOutput;

// A structure to hold the uniform for shaders
// Can be used to pass arbitrary data to the shader
typedef struct Uniform
{
	size_t frameCount;
	mat4d rotation;
} Uniform;

SRPContext srpContext;

void messageCallback(
	SRPMessageType type, SRPMessageSeverity severity, const char* sourceFunction,
	const char* message, void* userParameter
);
void vertexShader(SRPvsInput* in, SRPvsOutput* out);
void fragmentShader(SRPfsInput* in, SRPfsOutput* out);

int main()
{
	srpNewContext(&srpContext);
	srpContextSetMessageCallback(messageCallback);

	SRPFramebuffer* fb = srpNewFramebuffer(512, 512);

	const double R = 0.8;
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
			.nOutputVariables = 1,
			.outputVariablesInfo = (SRPVertexVariableInformation[]) {
				{.nItems = 3, .type = TYPE_DOUBLE}
			},
			.nBytesPerOutputVariables = sizeof(VSOutput)
		},
		.fs = &(SRPFragmentShader) {
			.shader = fragmentShader
		}
	};

	Window* window = newWindow(512, 512, "Rasterizer", false);

	while (window->running)
	{
		TIMER_START(frametime);

		// Part of the `mat` API. Again, you can use your own functions
		// (or an external math library) if you remove the `define`s at the
		// top of this file (`SRP_INCLUDE_...`)
		uniform.rotation = mat4dConstructRotate(0, 0, uniform.frameCount / 1000.);
		srpFramebufferClear(fb);
		srpDrawVertexBuffer(vb, fb, &shaderProgram, SRP_PRIM_TRIANGLES, 0, 3);

		windowPollEvents(window);
		windowPresent(window, fb);

		uniform.frameCount++;
		TIMER_STOP(frametime);
		printf(
			"Frametime: %li us; FPS: %lf; Framecount: %zu\n",
			TIMER_REPORT_US(frametime, long),
			1. / TIMER_REPORT_S(frametime, double),
			uniform.frameCount
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


void vertexShader(SRPvsInput* in, SRPvsOutput* out)
{
	Vertex* pVertex = (Vertex*) in->pVertex;
	Uniform* pUniform = (Uniform*) in->uniform;
	VSOutput* pOutVars = (VSOutput*) out->pOutputVariables;

	vec3d* inPosition = &pVertex->position;
	vec4d* outPosition = (vec4d*) out->position;
	*outPosition = (vec4d) {
		inPosition->x, inPosition->y, inPosition->z, 1.0
	};
	// Transform the position vector by the rotation matrix from uniform
	*outPosition = mat4dMultiplyVec4d(&pUniform->rotation, *outPosition);

	// Transform the color values just for fun
	pOutVars->color.x = pVertex->color.x + sin(pUniform->frameCount * 2.5e-3) * 0.3;
	pOutVars->color.y = pVertex->color.y + sin(pUniform->frameCount * 0.5e-3) * 0.1;
	pOutVars->color.z = pVertex->color.z + sin(pUniform->frameCount * 5e-3) * 0.5;
}

void fragmentShader(SRPfsInput* in, SRPfsOutput* out)
{
	VSOutput* interpolated = (VSOutput*) in->interpolated;
	vec4d* outColor = (vec4d*) out->color;
	outColor->x = interpolated->color.x;
	outColor->y = interpolated->color.y;
	outColor->z = interpolated->color.z;
	outColor->w = 1.;
}

