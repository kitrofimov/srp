#define SRP_INCLUDE_VEC
#define SRP_INCLUDE_MAT

#include <assert.h>
#include <srp/srp.h>
#include "save.h"

typedef struct Vertex
{
    vec3 position;
    vec3 color;
} Vertex;

SRPContext srpContext;

void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out);
void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out);

int main(int argc, char** argv)
{
    assert(argc >= 2);
    const char* outputPath = argv[1];

    Vertex data[] = {
        { .position = { -0.5, -0.5, 0. } },
        { .position = { -0.5,  0.5, 0. } },
        { .position = {  0.5,  0.5, 0. } },
        { .position = {  0.5, -0.5, 0. } },
    };

    SRPShaderProgram shaderProgram = {
        .uniform = NULL,
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
    SRPFramebuffer* fb = srpNewFramebuffer(512, 512);

    SRPVertexBuffer* vb = srpNewVertexBuffer();
    srpVertexBufferCopyData(vb, sizeof(Vertex), sizeof(data), data);

    srpFramebufferClear(fb);
    srpDrawVertexBuffer(vb, fb, &shaderProgram, SRP_PRIM_LINE_STRIP, 0, 4);

    int ok = saveFramebufferToImage(fb, outputPath);

    srpFreeVertexBuffer(vb);
    srpFreeFramebuffer(fb);

    return ok ? 0 : 1;
}

void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out)
{
    Vertex* v = (Vertex*) in->vertex;
	vec3* inPos = &v->position;
	vec4* outPos = (vec4*) out->clipPosition;
	*outPos = (vec4) { inPos->x, inPos->y, inPos->z, 1. };
}

void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out)
{
    out->color[0] = 1.;
    out->color[1] = 1.;
    out->color[2] = 1.;
    out->color[3] = 1.;
}
