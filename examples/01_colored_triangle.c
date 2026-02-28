// Do not add these `define`s if you don't want to include 
// `vec` and `mat` types and functions
#define SRP_INCLUDE_VEC
#define SRP_INCLUDE_MAT

#include <stdio.h>
#include <srp/srp.h>  // the only header you need to include
#include "window.h"
#include "framelimiter.h"
#include "rad.h"

// A structure to hold initial vertex data, stored in `SRPVertexBuffer`
// Is not necessary, just convenient to use
typedef struct Vertex
{
	vec3 position;
	vec3 color;
} Vertex;

// A structure to hold custom outputs from vertex shader, these will be
// interpolated inside the primitive
typedef struct VSOutput
{
	vec3 color;
} VSOutput;

// Library context, you should always define *and initialize* (see later) this!
SRPContext srpContext;

// Message callback: this is called for every error or warning
// Is not necessary
void messageCallback(
	SRPMessageType type, SRPMessageSeverity severity, const char* sourceFunction,
	const char* message, void* userParameter
);

// Vertex and fragment shaders, these should be always defined
void vertexShader(SRPvsInput* in, SRPvsOutput* out);
void fragmentShader(SRPfsInput* in, SRPfsOutput* out);

int main()
{
	// Initializing the context
	srpNewContext(&srpContext);
	srpContextSetMessageCallback(messageCallback);

	// Framebuffer object is necessary and stores the depth and
	// color (RGBA8888) buffers
	SRPFramebuffer* fb = srpNewFramebuffer(512, 512);

	// Initializing vertex and index data, computations made here (sin, cos, etc.
	// are only to build an quilateral triangle
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

	// Creating the vertex buffer object, it is similar to OpenGL's VBO
	SRPVertexBuffer* vb = srpNewVertexBuffer();
	srpVertexBufferCopyData(vb, sizeof(Vertex), sizeof(data), data);

	// Shader program is not actually a program, but named like this to
	// be similar to OpenGL
	SRPShaderProgram shaderProgram = {
		.uniform = NULL,
		.vs = &(SRPVertexShader) {
			.shader = vertexShader,
			// This stores the information about vertex shader's output variables
			// that is necessary to interpolate them inside the primitive
			.nOutputVariables = 1,
			.outputVariablesInfo = (SRPVertexVariableInformation[]) {
				{.nItems = 3, .type = SRP_FLOAT}
			},
			.nBytesPerOutputVariables = sizeof(VSOutput)
		},
		.fs = &(SRPFragmentShader) {
			.shader = fragmentShader,
			.doesOverwriteDepth = false
		}
	};

	// Is not a part of the library, this is a part of the example
	// See examples/utility/window.* and examples/utility/framelimiter.*
	Window* window = newWindow(512, 512, "Rasterizer", false);
	FrameLimiter limiter;
	frameLimiterInit(&limiter, 144.);

	// Main rendering loop
	size_t frameCount = 0;
	while (window->running)
	{
		frameLimiterBegin(&limiter);

		float renderTime = 0.;
		TIME_SECTION(renderTime, {
			// Clear the framebuffer and draw the index buffer as triangles
			srpFramebufferClear(fb);
			srpDrawVertexBuffer(vb, fb, &shaderProgram, SRP_PRIM_TRIANGLES, 0, 3);
		});

		windowPollEvents(window);
		windowPresent(window, fb);

		float frameTime = frameLimiterEnd(&limiter);
		frameCount++;

		if (frameCount % 100 == 0)
			printf(
				"Frametime: %5.3f ms; Rendering: %5.3f ms; FPS: %6.2f; RPS: %6.2f\n",
				frameTime * 1000., renderTime * 1000., 1. / frameTime, 1. / renderTime
			);
	}

	// Destroy objects
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
	// Cast the opaque input pointers to known types to make computations easier
	Vertex* pVertex = (Vertex*) in->pVertex;
	VSOutput* pOutVars = (VSOutput*) out->pOutputVariables;

	// `out->position` is defined as `float[4]`, but we cast it to `vec4*`
	// `vec` structures are tightly packed, so it is safe to cast float/float
	// arrays to vecXf/vecXd and vice versa!
	vec3* inPosition = &pVertex->position;
	vec4* outPosition = (vec4*) out->position;
	*outPosition = (vec4) {
		inPosition->x, inPosition->y, inPosition->z, 1.0
	};
	pOutVars->color = pVertex->color;

	// What we have done is just copied the inputs to the outputs
	// The simplest vertex shader possible!
}

void fragmentShader(SRPfsInput* in, SRPfsOutput* out)
{
	// Because the vertex shader's outputs are interpolated, in->interpolated
	// is an opaque pointer to VSOutput, so we cast it to this known type
	VSOutput* interpolated = (VSOutput*) in->interpolated;

	// See `vertexShader` comments
	vec4* outColor = (vec4*) out->color;
	outColor->x = interpolated->color.x;
	outColor->y = interpolated->color.y;
	outColor->z = interpolated->color.z;
	outColor->w = 1.;
}

// See the complete API reference by building Doxygen documentation
// (see README)

