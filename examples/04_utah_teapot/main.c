#define SRP_INCLUDE_VEC
#define SRP_INCLUDE_MAT

#include <stdio.h>
#include <srp/srp.h>
#include "window.h"
#include "framelimiter.h"
#include "objparser.h"
#include "rad.h"

typedef struct Uniform
{
	size_t frameCount;
	mat4d model;
	mat4d view;
	mat4d projection;
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

	srpContextSetI(SRP_CONTEXT_FRONT_FACE, SRP_FRONT_FACE_CW);
	srpContextSetI(SRP_CONTEXT_CULL_FACE, SRP_CULL_FACE_BACK);

	srpContextSetI(SRP_CONTEXT_POLYGON_MODE, SRP_POLYGON_MODE_FILL);

	OBJMesh mesh;
	if (!loadOBJMesh("res/objects/utah_teapot.obj", &mesh))
	{
		fprintf(stderr, "Failed to load mesh!\n");
		return -1;
	}

	SRPVertexBuffer* vb = srpNewVertexBuffer();
	SRPIndexBuffer* ib = srpNewIndexBuffer();
	srpVertexBufferCopyData(vb, sizeof(OBJVertex), mesh.vertexCount * sizeof(OBJVertex), mesh.vertices);
	srpIndexBufferCopyData(ib, TYPE_UINT32, mesh.indexCount * sizeof(uint32_t), mesh.indices);

	Uniform uniform = {
		.model = mat4dConstructIdentity(),
		.view = mat4dConstructView(
			0, 1.75, -7,
			0, 0, 0,
			1, 1, 1
		),
		.projection = mat4dConstructPerspectiveProjection(-1, 1, -1, 1, 1, 10),
		.frameCount = 0
	};

	SRPShaderProgram shaderProgram = {
		.uniform = (SRPUniform*) &uniform,
		.vs = &(SRPVertexShader) {
			.shader = vertexShader,
			.nOutputVariables = 0,
			.outputVariablesInfo = NULL,
			.nBytesPerOutputVariables = 0
		},
		.fs = &(SRPFragmentShader) {
			.shader = fragmentShader,
			.doesOverwriteDepth = false
		}
	};

	SRPFramebuffer* fb = srpNewFramebuffer(512, 512);
	Window* window = newWindow(512, 512, "Rasterizer", false);
	FrameLimiter limiter;
	frameLimiterInit(&limiter, 144.);

	while (window->running)
	{
		frameLimiterBegin(&limiter);

		double renderTime = 0.;
		TIME_SECTION(renderTime, {
			uniform.model = mat4dConstructRotate(
				RAD(-90), uniform.frameCount / 200., 0
			);
			srpFramebufferClear(fb);
			srpDrawIndexBuffer(ib, vb, fb, &shaderProgram, SRP_PRIM_TRIANGLES, 0, mesh.indexCount);
		});

		windowPollEvents(window);
		windowPresent(window, fb);

		double frameTime = frameLimiterEnd(&limiter);
		uniform.frameCount++;

		if (uniform.frameCount % 100 == 0)
			printf(
				"Frametime: %5.3f ms; Rendering: %5.3f ms; FPS: %6.2f; RPS: %6.2f\n",
				frameTime * 1000., renderTime * 1000., 1. / frameTime, 1. / renderTime
			);
	}

	srpFreeVertexBuffer(vb);
	srpFreeIndexBuffer(ib);
	srpFreeFramebuffer(fb);
	freeOBJMesh(&mesh);
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
	OBJVertex* pVertex = (OBJVertex*) in->pVertex;
	Uniform* pUniform = (Uniform*) in->uniform;

	vec3d* inPosition = &pVertex->position;
	vec4d* outPosition = (vec4d*) out->position;
	*outPosition = (vec4d) {
		inPosition->x, inPosition->y, inPosition->z, 1.0
	};
	*outPosition = mat4dMultiplyVec4d(&pUniform->model, *outPosition);
	*outPosition = mat4dMultiplyVec4d(&pUniform->view, *outPosition);
	*outPosition = mat4dMultiplyVec4d(&pUniform->projection, *outPosition);
}

void fragmentShader(SRPfsInput* in, SRPfsOutput* out)
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
