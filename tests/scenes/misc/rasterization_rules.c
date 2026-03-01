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

typedef struct VSOutput
{
    vec3 color;
} VSOutput;

typedef struct Uniform
{
	mat4 rotation;
} Uniform;

SRPContext srpContext;

void vertexShader(SRPvsInput* in, SRPvsOutput* out);
void fragmentShader(SRPfsInput* in, SRPfsOutput* out);

int main(int argc, char** argv)
{
    assert(argc >= 2);
    const char* outputPath = argv[1];

    // Two bounding triangles forming a square
    Vertex data[] = {
        { .position = { -0.5, -0.5, 0. }, .color = { 1., 0., 0. } },
        { .position = {  0.5,  0.5, 0. }, .color = { 0., 1., 0. } },
        { .position = { -0.5,  0.5, 0. }, .color = { 0., 0., 1. } },

        { .position = {  0.5,  0.5, 0. }, .color = { 1., 1., 0. } },
        { .position = {  0.5, -0.5, 0. }, .color = { 1., 0., 1. } },
        { .position = { -0.5, -0.5, 0. }, .color = { 0., 1., 1. } }
    };

    Uniform uniform = {
        .rotation = mat4ConstructRotate(0., 0., 1.5)
    };

    SRPShaderProgram shaderProgram = {
        .uniform = (SRPUniform*) &uniform,
        .vs = &(SRPVertexShader) {
            .shader = vertexShader,
            .nVaryings = 1,
            .varyingsInfo = (SRPVaryingInfo[]) {
                { .nItems = 3, .type = SRP_FLOAT }
            },
            .varyingsSize = sizeof(VSOutput)
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
    srpDrawVertexBuffer(vb, fb, &shaderProgram, SRP_PRIM_TRIANGLES, 0, 6);

    int ok = saveFramebufferToImage(fb, outputPath);

    srpFreeVertexBuffer(vb);
    srpFreeFramebuffer(fb);

    return ok ? 0 : 1;
}

void vertexShader(SRPvsInput* in, SRPvsOutput* out)
{
    Vertex* v = (Vertex*) in->vertex;
    Uniform* u = (Uniform*) in->uniform;
    VSOutput* o = (VSOutput*) out->varyings;

	vec3* inPos = &v->position;
	vec4* outPos = (vec4*) out->position;
	*outPos = (vec4) { inPos->x, inPos->y, inPos->z, 1. };
	*outPos = mat4MultiplyVec4(&u->rotation, *outPos);
    o->color = v->color;
}

void fragmentShader(SRPfsInput* in, SRPfsOutput* out)
{
    VSOutput* i = (VSOutput*) in->varyings;

    vec4* color = (vec4*) out->color;
    color->x = i->color.x;
    color->y = i->color.y;
    color->z = i->color.z;
    color->w = 1.;
}
