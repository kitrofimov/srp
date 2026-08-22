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

SRPContext srpContext;

void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out);
void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out);

int main(int argc, char** argv)
{
    assert(argc >= 2);
    const char* outputPath = argv[1];

    // Two overlapping triangles on different depths
    Vertex data[] = {
        // Back, red
        { .position = VEC3(-0.5, -0.5, 0.5), .color = VEC3(1., 0., 0.) },
        { .position = VEC3( 0.5, -0.5, 0.5), .color = VEC3(1., 0., 0.) },
        { .position = VEC3( 0,    0.5, 0.5), .color = VEC3(1., 0., 0.) },

        // Front, blue
        { .position = VEC3(-0.5,  0.5, -0.5), .color = VEC3(0., 0., 1.) },
        { .position = VEC3( 0,   -0.5, -0.5), .color = VEC3(0., 0., 1.) },
        { .position = VEC3( 0.5,  0.5, -0.5), .color = VEC3(0., 0., 1.) }
    };

    SRPShaderProgram shaderProgram = {
        .uniform = NULL,
        .vs = &(SRPVertexShader) {
            .shader = vertexShader,
            .nVaryings = 1,
            .varyingsInfo = (SRPVaryingInfo[]) {{
                .interpolationMode = SRP_INTERPOLATION_MODE_PERSPECTIVE,
                .nItems = 3,
                .type = SRP_FLOAT
            }},
            .varyingsSize = sizeof(VSOutput)
        },
        .fs = &(SRPFragmentShader) {
            .shader = fragmentShader,
            .mayOverwriteDepth = false
        }
    };

    srpNewContext(&srpContext);
    srpDepthTest(true);
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

void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out)
{
	Vertex* pVertex = (Vertex*) in->vertex;
	VSOutput* pOutVars = (VSOutput*) out->varyings;

	vec3* inPosition = &pVertex->position;
	vec4* outPosition = (vec4*) out->clipPosition;
	*outPosition = VEC4_FROM_VEC3(*inPosition, 1.);

	pOutVars->color = pVertex->color;
}

void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out)
{
    VSOutput* i = (VSOutput*) in->varyings;
    vec4* color = (vec4*) out->color;
    *color = VEC4_FROM_VEC3(i->color, 1);
}
