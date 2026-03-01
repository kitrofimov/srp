#define SRP_INCLUDE_VEC
#define SRP_INCLUDE_MAT

#include <assert.h>
#include <srp/srp.h>
#include "save.h"

typedef struct Vertex
{
    vec3 position;
    uint8_t color;
} Vertex;

typedef struct VSOutput
{
    uint8_t color;
} VSOutput;

typedef struct Uniform
{
	mat4 model;
} Uniform;

SRPContext srpContext;

void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out);
void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out);

int main(int argc, char** argv)
{
    assert(argc >= 2);
    const char* outputPath = argv[1];

    Vertex data[] = {
        { .position = { -0.5, -0.5, 0. }, .color = 0 },
        { .position = {  0.5, -0.5, 0. }, .color = 1 },
        { .position = {  0. ,  0.5, 0. }, .color = 2 },
    };

    Uniform uniform = {0};
    SRPShaderProgram shaderProgram = {
        .uniform = (SRPUniform*) &uniform,
        .vs = &(SRPVertexShader) {
            .shader = vertexShader,
            .nVaryings = 1,
			.varyingsInfo = (SRPVaryingInfo[]) {{
				.nItems = 3,
				.type = SRP_FLOAT,
				.interpolationMode = SRP_INTERPOLATION_MODE_FLAT
			}},
            .varyingsSize = sizeof(VSOutput)
        },
        .fs = &(SRPFragmentShader) {
            .shader = fragmentShader,
            .mayOverwriteDepth = false
        }
    };

    srpNewContext(&srpContext);
    SRPFramebuffer* fb = srpNewFramebuffer(512, 512);
    srpFramebufferClear(fb);

    SRPVertexBuffer* vb = srpNewVertexBuffer();
    srpVertexBufferCopyData(vb, sizeof(Vertex), sizeof(data), data);

    // Expected: left, red
    srpContextSetI(SRP_CONTEXT_PROVOKING_VERTEX_MODE, SRP_PROVOKING_VERTEX_FIRST);
    uniform.model = mat4ConstructTRS(-0.5, 0, 0,   0, 0, 0,   0.5, 0.5, 0.5);
    srpDrawVertexBuffer(vb, fb, &shaderProgram, SRP_PRIM_TRIANGLES, 0, 3);

    // Expected: right, blue
    srpContextSetI(SRP_CONTEXT_PROVOKING_VERTEX_MODE, SRP_PROVOKING_VERTEX_LAST);
    uniform.model = mat4ConstructTRS( 0.5, 0, 0,   0, 0, 0,   0.5, 0.5, 0.5);
    srpDrawVertexBuffer(vb, fb, &shaderProgram, SRP_PRIM_TRIANGLES, 0, 3);

    int ok = saveFramebufferToImage(fb, outputPath);

    srpFreeVertexBuffer(vb);
    srpFreeFramebuffer(fb);

    return ok ? 0 : 1;
}

void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out)
{
    Vertex* v = (Vertex*) in->vertex;
    Uniform* u = (Uniform*) in->uniform;
    VSOutput* o = (VSOutput*) out->varyings;

	vec3* inPos = &v->position;
	vec4* outPos = (vec4*) out->clipPosition;
	*outPos = (vec4) { inPos->x, inPos->y, inPos->z, 1. };
	*outPos = mat4MultiplyVec4(&u->model, *outPos);
    o->color = v->color;
}

void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out)
{
    VSOutput* v = (VSOutput*) in->varyings;
    vec4* color = (vec4*) out->color;

    if (v->color == 0)
        *color = (vec4) { 1, 0, 0, 1 };
    else if (v->color == 1)
        *color = (vec4) { 0, 1, 0, 1 };
    else if (v->color == 2)
        *color = (vec4) { 0, 0, 1, 1 };
}
