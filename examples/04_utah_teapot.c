#define SRP_INCLUDE_VEC
#define SRP_INCLUDE_MAT

#include <stdio.h>
#include <srp/srp.h>
#include "window.h"
#include "framelimiter.h"
#include "objparser.h"
#include "rad.h"

typedef struct VSOutput
{
	vec3 normal;
	vec3 fragPos;
} VSOutput;

typedef struct Material
{
	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} Material;

typedef struct Light
{
	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	vec3 direction;
} Light;

typedef struct Uniform
{
	size_t frameCount;
	mat4 model;
	mat4 view;
	mat4 projection;
	Material material;
	Light light;
	vec3 viewPos;
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
	srpSetMessageCallback((SRPMessageCallback) {
		.func = messageCallback,
		.userParameter = NULL
	});

	srpRasterFrontFace(SRP_WINDING_CW);
	srpRasterCullFace(SRP_FACE_BACK);
	srpRasterPolygonMode(SRP_POLYGON_MODE_FILL);
	srpDepthTest(true);

	OBJMesh mesh;
	if (!loadOBJMesh("res/objects/utah_teapot.obj", &mesh))
	{
		fprintf(stderr, "Failed to load mesh!\n");
		return -1;
	}

	SRPVertexBuffer* vb = srpNewVertexBuffer();
	SRPIndexBuffer* ib = srpNewIndexBuffer();
	srpVertexBufferCopyData(vb, sizeof(OBJVertex), mesh.vertexCount * sizeof(OBJVertex), mesh.vertices);
	srpIndexBufferCopyData(ib, SRP_UINT32, mesh.indexCount * sizeof(uint32_t), mesh.indices);

	vec3 viewPos = (vec3) { 0, 1.75, -7 };
	Uniform uniform = {
		.model = mat4ConstructIdentity(),
		.view = mat4ConstructView(
			viewPos.x, viewPos.y, viewPos.z,
			0, 0, 0,
			1, 1, 1
		),
		.projection = mat4ConstructPerspectiveProjection(-1, 1, -1, 1, 1, 10),
		.frameCount = 0,
		.material = (Material) {
			.ambient  = (vec3) { 1,   0.5, 0.31 },
			.diffuse  = (vec3) { 1,   0.5, 0.31 },
			.specular = (vec3) { 0.5, 0.5, 0.5  },
			.shininess = 2
		},
		.light = (Light) {
			.ambient  = (vec3) { 0.1, 0.1, 0.1 },
			.diffuse  = (vec3) { 0.5, 0.5, 0.5 },
			.specular = (vec3) { 0.3, 0.3, 0.3 },
			.direction = (vec3) { -1, -1, 1 }
		},
		.viewPos = viewPos
	};

	SRPShaderProgram shaderProgram = {
		.uniform = (SRPUniform*) &uniform,
		.vs = &(SRPVertexShader) {
			.shader = vertexShader,
			.nVaryings = 2,
			.varyingsInfo = (SRPVaryingInfo[]) {
				{
					.interpolationMode = SRP_INTERPOLATION_MODE_PERSPECTIVE,
					.nItems = 3,
					.type = SRP_FLOAT
				},
				{
					.interpolationMode = SRP_INTERPOLATION_MODE_PERSPECTIVE,
					.nItems = 3,
					.type = SRP_FLOAT
				}
			},
			.varyingsSize = sizeof(VSOutput)
		},
		.fs = &(SRPFragmentShader) {
			.shader = fragmentShader,
			.mayOverwriteDepth = false
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
			uniform.model = mat4ConstructRotate(
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


void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out)
{
	OBJVertex* pVertex = (OBJVertex*) in->vertex;
	Uniform* pUniform = (Uniform*) in->uniform;
	VSOutput* v = (VSOutput*) out->varyings;

	vec3* inPosition = &pVertex->position;
	vec4* outPosition = (vec4*) out->clipPosition;
	*outPosition = (vec4) { inPosition->x, inPosition->y, inPosition->z, 1. };
	*outPosition = mat4MultiplyVec4(&pUniform->model, *outPosition);
	v->fragPos = *(vec3*) outPosition;
	*outPosition = mat4MultiplyVec4(&pUniform->view, *outPosition);
	*outPosition = mat4MultiplyVec4(&pUniform->projection, *outPosition);

	vec4 worldNormal = mat4MultiplyVec4(
		&pUniform->model,
		(vec4) { pVertex->normal.x, pVertex->normal.y, pVertex->normal.z, 0.f }
	);
	v->normal.x = worldNormal.x;
	v->normal.y = worldNormal.y;
	v->normal.z = worldNormal.z;
}

void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out)
{    
	Uniform* u = (Uniform*) in->uniform;
	VSOutput* v = (VSOutput*) in->varyings;

	Material* m = &u->material;
	Light* l = &u->light;

	vec3 ambient = vec3MultiplyVec3(l->ambient, m->ambient);
  	
    vec3 norm = vec3Normalize(v->normal);
    float diff = fmaxf(vec3DotProduct(norm, vec3Negate(l->direction)), 0.);
	vec3 diffuse = vec3MultiplyVec3(l->diffuse, vec3MultiplyScalar(m->diffuse, diff));
    
    vec3 viewDir = vec3Normalize(vec3Subtract(u->viewPos, v->fragPos));
    vec3 reflectDir = vec3Reflect(l->direction, norm);  
    float spec = powf(fmaxf(vec3DotProduct(viewDir, reflectDir), 0.0), m->shininess);
	vec3 specular = vec3MultiplyVec3(l->specular, vec3MultiplyScalar(m->specular, spec));
        
    vec3 result = vec3Add(vec3Add(ambient, diffuse), specular);
	*(vec3*) out->color = result;
	out->color[3] = 1.;
}
